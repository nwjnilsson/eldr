#include <eldr/core/core.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/descriptorallocator.hpp> // SceneData
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

#include <eldr/vulkan/sceneresources.hpp>

#include <eldr/ext/fastgltf.hpp>

// Specialize vector types for use with fastgltf. Just simple Vector<float, s>
#define EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(Name, Size)                     \
  template <>                                                                  \
  struct ElementTraits<Name<float, Size>>                                      \
    : ElementTraitsBase<Name<float, Size>, AccessorType::Vec##Size, float> {};
NAMESPACE_BEGIN(fastgltf)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Point, 2)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Point, 3)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Vector, 3)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Normal, 3)
EL_DECLARE_FASTGLTF_ELEMENT_TRAIT_SPEC(eldr::core::Color, 4)
NAMESPACE_END(fastgltf)

// TODO: use rapidobj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace eldr::core;

NAMESPACE_BEGIN(eldr::render)
//------------------------------------------------------------------------------
// Scene node
//------------------------------------------------------------------------------
void SceneNode::refreshTransform(const Transform4f& parent_transform)
{
  world_transform = parent_transform * local_transform;
  for (auto c : children) {
    c->refreshTransform(world_transform);
  }
}

void SceneNode::draw(const RenderObject::Matrix4f& top_matrix,
                     DrawContext&                  ctx) const
{
  for (auto& c : children)
    c->draw(top_matrix, ctx);
}

void SceneNode::map(std::function<void(SceneNode*)> func)
{
  func(this);
  for (auto& c : children)
    c->map(func);
}

//------------------------------------------------------------------------------
// Mesh node
//------------------------------------------------------------------------------
EL_VARIANT void MeshNode<Float, Spectrum>::draw(const Transform4f& top_matrix,
                                                DrawContext&       ctx) const
{
  const Transform4f node_transform{ top_matrix * world_transform };

  for (const auto& s : mesh->surfaces()) {
    const RenderObject obj{
      .index_count = s.count,
      .first_index = s.start_index,
      .material    = s.material.get(),
      //.transform   = node_matrix,
    };
    ctx.opaque_surfaces.push_back(obj);
  }
  // Draw recursively
  SceneNode::draw(top_matrix, ctx);
}

//------------------------------------------------------------------------------
// Scene
//------------------------------------------------------------------------------
EL_VARIANT void Scene<Float, Spectrum>::draw(DrawContext& ctx) const
{
  ctx.opaque_surfaces.clear();
  const Transform4f top_matrix{ 1.f };
  for (auto& n : top_nodes_) {
    n->draw(top_matrix, ctx);
  }
}

EL_VARIANT
std::optional<std::shared_ptr<Scene<Float, Spectrum>>>
Scene<Float, Spectrum>::loadGltf(const vk::VulkanEngine& engine,
                                 std::filesystem::path   file_path)
{
  namespace fg = fastgltf;
  Log(Trace, "Loading glTF: {}", file_path.c_str());

  auto data{ fg::GltfDataBuffer::FromPath(file_path) };
  if (auto error = data.error(); error != fg::Error::None) {
    Log(Error,
        "Failed to create GLTF data buffer: {} - {}",
        fg::getErrorName(error),
        fg::getErrorMessage(error));
    return std::nullopt;
  }

  fg::Parser     parser{};
  constexpr auto gltf_options{ fg::Options::DontRequireValidAssetMember |
                               fg::Options::AllowDouble |
                               fg::Options::LoadExternalBuffers };
  // fg::Options::LoadExternalImages;

  fg::Asset gltf;
  auto      type = fg::determineGltfFileType(data.get());
  if (type == fg::GltfType::glTF) {
    auto load =
      parser.loadGltf(data.get(), file_path.parent_path(), gltf_options);
    auto error = load.error();
    if (error == fg::Error::None) {
      gltf = std::move(load.get());
    }
    else {
      Log(Error,
          "Failed to load glTF: {} - {}",
          fg::getErrorName(error),
          fg::getErrorMessage(error));
      return std::nullopt;
    }
  }
  else if (type == fg::GltfType::GLB) {
    auto load =
      parser.loadGltfBinary(data.get(), file_path.parent_path(), gltf_options);
    const fg::Error error{ load.error() };
    if (error != fg::Error::None) {
      gltf = std::move(load.get());
    }
    else {
      Log(Error,
          "Failed to load glTF: {} - {}",
          fg::getErrorName(error),
          fg::getErrorMessage(error));
      return std::nullopt;
    }
  }
  else {
    Log(Error, "Failed to determine glTF container");
    return std::nullopt;
  }

  if (unlikely(gltf.materials.empty())) {
    Log(Error, "glTF file contains no materials, at least one is required");
    return std::nullopt;
  }
  else if (unlikely(gltf.meshes.empty())) {
    Log(Error, "glTF file contains no meshes, at least one is required");
    return std::nullopt;
  }
  else if (unlikely(gltf.nodes.empty())) {
    Log(Error, "glTF file contains no nodes, at least one is required");
    return std::nullopt;
  }

  //----------------------------------------------------------------------------
  // Load vulkan resources (images, samplers, materials)
  //----------------------------------------------------------------------------
  auto scene           = std::make_shared<Scene>();
  scene->vk_resources_ = engine.createSceneResources(gltf);

  //----------------------------------------------------------------------------
  // Load meshes
  //----------------------------------------------------------------------------
  std::vector<std::shared_ptr<Scene::Mesh>> meshes;
  for (fg::Mesh& mesh : gltf.meshes) {
    std::vector<uint32_t> indices; // currently not giving this to the mesh,
                                   // TODO: might be needed
    std::vector<Point3f>    vertices;
    std::vector<Point2f>    texcoords;
    std::vector<Color4f>    colors;
    std::vector<Normal3f>   normals;
    std::vector<GeoSurface> surfaces;
    surfaces.reserve(mesh.primitives.size());

    for (auto&& p : mesh.primitives) {
      GeoSurface surface;
      surface.start_index = static_cast<uint32_t>(indices.size());
      surface.count =
        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count);

      const size_t initial_vtx = vertices.size();
      // load indices
      {
        fg::Accessor& index_accessor =
          gltf.accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + index_accessor.count);

        fg::iterateAccessor<uint32_t>(gltf, index_accessor, [&](uint32_t idx) {
          indices.push_back(idx + initial_vtx);
        });
      }

      // load vertex vertices
      {
        // POSITION attribute is required to be present
        fg::Accessor& pos_accessor =
          gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
        const size_t new_size{ initial_vtx + pos_accessor.count };
        vertices.reserve(new_size);
        texcoords.reserve(new_size);
        colors.reserve(new_size);
        normals.reserve(new_size);

        fg::iterateAccessor<Point3f>(gltf, pos_accessor, [&](Point3f p) {
          vertices.push_back(p);
          texcoords.push_back(Point2f{ 0.f, 0.f });
          colors.push_back(Color4f{ 1.f });
          normals.push_back(Normal3f{ 1, 0, 0 });
        });
      }

      // load vertex normals
      auto attr_normals{ p.findAttribute("NORMAL") };
      if (attr_normals != p.attributes.end()) {
        fg::iterateAccessorWithIndex<Normal3f>(
          gltf,
          gltf.accessors[attr_normals->accessorIndex],
          [&](Normal3f n, size_t index) { normals[initial_vtx + index] = n; });
      }

      // load UVs
      auto attr_uv{ p.findAttribute("TEXCOORD_0") };
      if (attr_uv != p.attributes.end()) {
        fg::iterateAccessorWithIndex<Point2f>(
          gltf,
          gltf.accessors[attr_uv->accessorIndex],
          [&](Point2f p, size_t index) {
            texcoords[initial_vtx + index] = { p.x, p.y };
          });
      }
      // load vertex colors
      auto attr_colors{ p.findAttribute("COLOR_0") };
      if (attr_colors != p.attributes.end()) {
        fg::iterateAccessorWithIndex<Color4f>(
          gltf,
          gltf.accessors[attr_colors->accessorIndex],
          [&](Color4f c, size_t index) { colors[initial_vtx + index] = c; });
      }

      if (p.materialIndex.has_value()) {
        surface.material =
          scene->vk_resources_->materials[p.materialIndex.value()];
      }
      else {
        Log(Warn, "Missing primitive material index, using index 0 instead.");
        surface.material = scene->vk_resources_->materials[0];
      }
      surfaces.push_back(surface);
    }

    // display the vertex normals
    // constexpr bool override_colors = true;
    // if (override_colors) {
    //   for (size_t i = 0; i < colors.size(); ++i) {
    //     colors[i] = Color4f{ normals[i], 1.f };
    //   }
    // }
    auto newmesh = std::make_shared<Mesh>(mesh.name,
                                          std::move(vertices),
                                          std::move(texcoords),
                                          std::move(colors),
                                          std::move(normals),
                                          std::move(surfaces));
    meshes.emplace_back(newmesh);
    const auto res =
      scene->meshes_.insert(std::make_pair(mesh.name, std::move(newmesh)));
    if (unlikely(not res.second)) {
      Log(Warn, "Scene contains duplicate mesh name ({}).", mesh.name);
    }
  }

  //----------------------------------------------------------------------------
  // Load nodes
  //----------------------------------------------------------------------------
  std::vector<std::shared_ptr<SceneNode>> nodes;
  for (fg::Node& node : gltf.nodes) {
    std::shared_ptr<SceneNode> scene_node;
    if (node.meshIndex.has_value()) {
      auto p_mesh  = std::make_shared<MeshNode<Float, Spectrum>>();
      p_mesh->mesh = meshes[*node.meshIndex];
      scene_node   = std::move(p_mesh);
    }
    else {
      scene_node = std::make_shared<SceneNode>();
    }
    nodes.push_back(scene_node);
    const auto res =
      scene->nodes_.insert(std::make_pair(node.name, scene_node));
    if (unlikely(not res.second)) {
      Log(Warn, "Scene contains duplicate node name ({}).", node.name);
    }

    std::visit(
      fg::visitor{ [&](fg::math::fmat4x4 matrix) {
                    for (size_t i = 0; i < matrix.rows(); ++i) {
                      for (size_t j = 0; i < matrix.columns(); ++j) {
                        scene_node->local_transform[i][j] = matrix[i][j];
                      }
                    }
                  },
                   [&](fg::TRS transform) {
                     Vector3f tl{ transform.translation[0],
                                  transform.translation[1],
                                  transform.translation[2] };
                     Quat4f   rot{ transform.rotation[3],
                                 transform.rotation[0],
                                 transform.rotation[1],
                                 transform.rotation[2] };
                     Vector3f sc{ transform.scale[0],
                                  transform.scale[1],
                                  transform.scale[2] };

                     Transform4f tm{ glm::translate(Matrix4f{ 1.f }, tl) };
                     Transform4f rm{ glm::toMat4(rot) };
                     Transform4f sm{ glm::scale(Matrix4f{ 1.f }, sc) };

                     scene_node->local_transform = tm * rm * sm;
                   } },
      node.transform);
  }

  // run loop again to setup transform hierarchy
  for (size_t i = 0; i < gltf.nodes.size(); i++) {
    fg::Node&                   node{ gltf.nodes[i] };
    std::shared_ptr<SceneNode>& scene_node{ nodes[i] };

    for (auto& c : node.children) {
      scene_node->children.push_back(nodes[c]);
      nodes[c]->parent = scene_node;
    }
  }

  // find the top nodes, with no parents
  for (auto& node : nodes) {
    if (node->parent.lock() == nullptr) {
      scene->top_nodes_.push_back(node);
      node->refreshTransform(Transform4f{ 1.f });
    }
  }
  Log(Trace,
      "Loaded {} meshes, {} materials and {} nodes",
      scene->meshes_.size(),
      scene->vk_resources_->materials.size(),
      scene->nodes_.size());
  return scene;
}

// TODO: support MTL file and loading texture from such a reference
// template <typename T>
// std::optional<std::shared_ptr<Scene>>
// Scene::loadObj(std::filesystem::path file_path)
// {
//   Logger log{ requestLogger("mesh") };
//   Log(core::Trace"Loading OBJ: {}", file_path.c_str());
//   // TODO: use rapidobj instead and remove tinyobjloader submodule
//   tinyobj::attrib_t                attrib;
//   std::vector<tinyobj::shape_t>    shapes
//   std::vector<tinyobj::material_t> materials;
//   std::string                      warn, err;
//
//   // const char* env_p = std::getenv("ELDR_DIR");
//
//   if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
//                         file_path.c_str())) {
//     Log(core::Error"tinyobjloader error:\nwarn: {}\nerror: {}", warn, err);
//     return std::nullopt;
//   }
//
//   std::shared_ptr<Scene> scene{};
//
//   for (const auto& shape : shapes) {
//     std::vector<Point3f>    vertices;
//     std::vector<Point2f>    texcoords;
//     std::vector<Color4f>    colors;
//     std::vector<Vec3f>      normals;
//     std::vector<GeoSurface> surfaces;
//
//     // OBJ does not support submeshes so each surface starts on index 0
//     GeoSurface surface;
//     surface.start_index = 0;
//     surface.count       = static_cast<uint32_t>(shape.mesh.indices.size());
//
//     for (const auto& index : shape.mesh.indices) {
//       vertices.push_back({
//         attrib.vertices[3 * index.vertex_index],
//         attrib.vertices[3 * index.vertex_index + 1],
//         attrib.vertices[3 * index.vertex_index + 2],
//       });
//       texcoords.push_back({
//         attrib.texcoords[2 * index.texcoord_index],
//         // Invert y coordinate because OBJ assumes y=0 is bottom
//         // left while Vulkan says y=0 is top left
//         1.f - attrib.texcoords[2 * index.texcoord_index + 1],
//       });
//       normals.push_back({
//         attrib.normals[3 * index.normal_index],
//         attrib.normals[3 * index.normal_index + 1],
//         attrib.normals[3 * index.normal_index + 2],
//       });
//       colors.push_back({ attrib.colors[3 * index.vertex_index],
//                          attrib.colors[3 * index.vertex_index + 1],
//                          attrib.colors[3 * index.vertex_index + 2], 1.0f });
//     }
//
//     loaded_shapes.emplace_back(std::make_shared<Mesh>(
//       shape.name, std::move(vertices), std::move(texcoords),
//       std::move(colors), std::move(normals), std::move(surfaces)));
//   }
//   return loaded_shapes;
// }

EL_VARIANT
std::optional<std::shared_ptr<Scene<Float, Spectrum>>>
Scene<Float, Spectrum>::load(const vk::VulkanEngine& engine,
                             const SceneInfo&        scene_info)
{
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }

  std::filesystem::path filepath(env_p);
  filepath /= scene_info.model_path;

  if (not std::filesystem::is_regular_file(filepath)) {
    Throw("Scene file not found!");
  }

  // TODO: determine file type obj/gltf if obj is to be supported too
  auto scene = Scene::loadGltf(engine, filepath);

  return scene;
}
EL_INSTANTIATE_CLASS(Scene)
NAMESPACE_END(eldr::render)
