#include <eldr/core/exceptions.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

#include <spdlog/spdlog.h>

// TODO: use rapidobj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// Ignore warnings from fastgltf
#ifdef __GNUG__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wredundant-move"
// this doesn't quite work due to lack of gcc support, but looks like it's on
// the way
#  pragma GCC diagnostic ignored "-Wunknown-pragmas"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  include <fastgltf/core.hpp>
#  include <fastgltf/glm_element_traits.hpp>
#  include <fastgltf/tools.hpp>
#  include <fastgltf/types.hpp>
#  pragma GCC diagnostic pop
#endif
// TODO: corresponding thing for windows builds

namespace {
VkFilter extractFilter(fastgltf::Filter filter)
{
  switch (filter) {
    // nearest samplers
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
      return VK_FILTER_NEAREST;

    // linear samplers
    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
      return VK_FILTER_LINEAR;
  }
}

VkSamplerMipmapMode extractMipmapMode(fastgltf::Filter filter)
{
  switch (filter) {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
  }
}
} // namespace

namespace eldr::vk {
// TODO: move
struct SceneData {
  std::vector<vk::wr::Sampler> samplers;
  vk::DescriptorAllocator      descriptors;
  vk::wr::Buffer               material_buffer;
};
} // namespace eldr::vk

namespace eldr {
//------------------------------------------------------------------------------
// Scene node
//------------------------------------------------------------------------------
void SceneNode::refreshTransform(const Mat4f& parent_matrix)
{
  world_transform = parent_matrix * local_transform;
  for (auto c : children) {
    c->refreshTransform(world_transform);
  }
}

void SceneNode::draw(const Mat4f& top_matrix, DrawContext& ctx) const
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
void MeshNode::draw(const Mat4f& top_matrix, DrawContext& ctx) const
{
  const Mat4f node_matrix{ top_matrix * world_transform };

  for (auto& s : mesh->surfaces()) {
    const RenderObject obj{
      .index_count = s.count,
      .first_index = s.start_index,
      .material    = s.material.get(),
      .transform   = node_matrix,
    };
    ctx.opaque_surfaces.push_back(obj);
  }
  // Draw recursively
  SceneNode::draw(top_matrix, ctx);
}

//------------------------------------------------------------------------------
// Scene
//------------------------------------------------------------------------------
void Scene::draw(const Mat4f& top_matrix, DrawContext& ctx) const
{
  for (auto& n : top_nodes) {
    n->draw(top_matrix, ctx);
  }
}

std::optional<std::shared_ptr<Scene>> loadGltf(const vk::VulkanEngine& engine,
                                               std::filesystem::path file_path)
{
  ELDR_IMPORT_CORE_TYPES()
  Logger log{ requestLogger("mesh") };

  namespace fg = fastgltf;
  fmt::print("Loading GLTF: {}", file_path.c_str());

  std::shared_ptr<Scene> p_scene{ std::make_shared<Scene>() };
  // p_scene->creator               = engine;
  Scene& scene{ *p_scene.get() };

  fg::Parser     parser{};
  constexpr auto gltf_options{ fg::Options::DontRequireValidAssetMember |
                               fg::Options::AllowDouble |
                               fg::Options::LoadExternalBuffers };
  // fg::Options::LoadExternalImages;

  auto data{ fg::GltfDataBuffer::FromPath(file_path) };
  if (auto error = data.error(); error != fg::Error::None) {
    log->error("Failed to create GLTF data buffer: {} - {}",
               fg::getErrorName(error), fg::getErrorMessage(error));
    return {};
  }

  fg::Asset gltf;
  auto      type = fg::determineGltfFileType(data.get());
  if (type == fg::GltfType::glTF) {
    auto load{ parser.loadGltf(data.get(), file_path.parent_path(),
                               gltf_options) };
    auto error{ load.error() };
    if (error == fg::Error::None) {
      gltf = std::move(load.get());
    }
    else {
      log->error("Failed to load glTF: {} - {}", fg::getErrorName(error),
                 fg::getErrorMessage(error));
      return {};
    }
  }
  else if (type == fg::GltfType::GLB) {
    auto load{ parser.loadGltfBinary(data.get(), file_path.parent_path(),
                                     gltf_options) };
    auto error{ load.error() };
    if (error != fg::Error::None) {
      gltf = std::move(load.get());
    }
    else {
      log->error("Failed to load glTF: {} - {}", fg::getErrorName(error),
                 fg::getErrorMessage(error));
      return {};
    }
  }
  else {
    log->error("Failed to determine glTF container");
    return {};
  }

  scene.vk_scene_data = std::make_shared<vk::SceneData>();

  // load samplers
  for (fg::Sampler& sampler : gltf.samplers) {
    VkFilter            min_filter{ extractFilter(
      sampler.minFilter.value_or(fg::Filter::Nearest)) };
    VkFilter            mag_filter{ extractFilter(
      sampler.magFilter.value_or(fg::Filter::Nearest)) };
    VkSamplerMipmapMode mipmap_mode{ extractMipmapMode(fg::Filter::Nearest) };
    scene.vk_scene_data->samplers.emplace_back(
      engine.device(), min_filter, mag_filter, mipmap_mode, VK_LOD_CLAMP_NONE);
  }

  std::vector<std::shared_ptr<Mesh>>      meshes;
  std::vector<std::shared_ptr<SceneNode>> nodes;
  std::vector<vk::wr::Texture>            images;
  std::vector<std::shared_ptr<Material>>  materials;

  images.resize(gltf.images.size(), engine.errorImage());

  const size_t mat_buffer_size{
    sizeof(GltfMetallicRoughness::MaterialConstants) * gltf.materials.size()
  };

  scene.vk_scene_data->material_buffer =
    vk::wr::Buffer{ engine.device(), "Material buffer", mat_buffer_size,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VMA_MEMORY_USAGE_CPU_TO_GPU };

  int data_index{ 0 };
  std::vector<GltfMetallicRoughness::MaterialConstants>
    scene_material_constants;
  //----------------------------------------------------------------------------
  // Load materials
  //----------------------------------------------------------------------------
  for (fg::Material& mat : gltf.materials) {
    std::shared_ptr<Material> material{ std::make_shared<Material>() };
    materials.push_back(material);
    scene.materials[mat.name.c_str()] = material;

    GltfMetallicRoughness::MaterialConstants constants;
    constants.color_factors.x = mat.pbrData.baseColorFactor[0];
    constants.color_factors.y = mat.pbrData.baseColorFactor[1];
    constants.color_factors.z = mat.pbrData.baseColorFactor[2];
    constants.color_factors.w = mat.pbrData.baseColorFactor[3];

    constants.metal_rough_factors.x = mat.pbrData.metallicFactor;
    constants.metal_rough_factors.y = mat.pbrData.roughnessFactor;

    // write material parameters to buffer
    scene_material_constants.push_back(constants);

    MaterialPass pass_type = MaterialPass::MainColor;
    if (mat.alphaMode == fg::AlphaMode::Blend) {
      pass_type = MaterialPass::Transparent;
    }

    GltfMetallicRoughness::MaterialResources material_resources{
      // default the material textures
      .color_texture       = &engine.whiteImage(),
      .color_sampler       = &engine.defaultSamplerLinear(),
      .metal_rough_texture = &engine.whiteImage(),
      .metal_rough_sampler = &engine.defaultSamplerLinear(),

      // set the uniform buffer for the material data
      .data_buffer = &scene.vk_scene_data->material_buffer,
      .data_buffer_offset =
        data_index * sizeof(GltfMetallicRoughness::MaterialConstants),
    };
    // grab textures from gltf file
    if (mat.pbrData.baseColorTexture.has_value()) {
      size_t img =
        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
          .imageIndex.value();
      size_t sampler =
        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex]
          .samplerIndex.value();

      material_resources.color_texture = &images[img];
      material_resources.color_sampler =
        &scene.vk_scene_data->samplers[sampler];
    }
    // build material
    material->data = engine.metalRoughMaterial().writeMaterial(
      engine.device(), pass_type, material_resources,
      scene.vk_scene_data->descriptors);

    data_index++;
  }

  scene.vk_scene_data->material_buffer
    .uploadData<GltfMetallicRoughness::MaterialConstants>(
      scene_material_constants);

  //----------------------------------------------------------------------------
  // Load meshes
  //----------------------------------------------------------------------------
  for (fg::Mesh& mesh : gltf.meshes) {
    const std::string       name{ mesh.name };
    std::vector<uint32_t>   indices;
    std::vector<Point3f>    positions;
    std::vector<Point2f>    texcoords;
    std::vector<Color4f>    colors;
    std::vector<Vec3f>      normals;
    std::vector<GeoSurface> surfaces;
    surfaces.reserve(mesh.primitives.size());

    for (auto&& p : mesh.primitives) {
      GeoSurface surface;
      surface.start_index = static_cast<uint32_t>(indices.size());
      surface.count =
        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count);
      surfaces.push_back(surface);

      const size_t initial_vtx = positions.size();
      // load indexes
      {
        fg::Accessor& index_accessor =
          gltf.accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + index_accessor.count);

        fg::iterateAccessor<uint32_t>(gltf, index_accessor, [&](uint32_t idx) {
          indices.push_back(idx + initial_vtx);
        });
      }

      // load vertex positions
      {
        // POSITION attribute is required to be present
        fg::Accessor& pos_accessor =
          gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
        const size_t new_size{ initial_vtx + pos_accessor.count };
        positions.reserve(new_size);
        texcoords.reserve(new_size);
        colors.reserve(new_size);
        normals.reserve(new_size);

        fg::iterateAccessor<Point3f>(gltf, pos_accessor, [&](Point3f p) {
          positions.push_back(p);
          texcoords.push_back({ 0, 0 });
          colors.push_back(Color4f{ 1.f });
          normals.push_back({ 1, 0, 0 });
        });
      }

      // load vertex normals
      auto attr_normals{ p.findAttribute("NORMAL") };
      if (attr_normals != p.attributes.end()) {
        fg::iterateAccessorWithIndex<Vec3f>(
          gltf, gltf.accessors[attr_normals->accessorIndex],
          [&](Vec3f n, size_t index) { normals[initial_vtx + index] = n; });
      }

      // load UVs
      auto attr_uv{ p.findAttribute("TEXCOORD_0") };
      if (attr_uv != p.attributes.end()) {
        fg::iterateAccessorWithIndex<Point2f>(
          gltf, gltf.accessors[attr_uv->accessorIndex],
          [&](Point2f p, size_t index) {
            texcoords[initial_vtx + index] = { p.x, p.y };
          });
      }

      // load vertex colors
      auto attr_colors{ p.findAttribute("COLOR_0") };
      if (attr_colors != p.attributes.end()) {
        fg::iterateAccessorWithIndex<Color4f>(
          gltf, gltf.accessors[attr_colors->accessorIndex],
          [&](Color4f c, size_t index) { colors[initial_vtx + index] = c; });
      }

      if (p.materialIndex.has_value())
        surface.material = materials[p.materialIndex.value()];
      else
        surface.material = materials[0];
      surfaces.push_back(surface);
    }

    // display the vertex normals
    // constexpr bool override_colors = true;
    // if (override_colors) {
    //   for (size_t i = 0; i < colors.size(); ++i) {
    //     colors[i] = Color4f{ normals[i], 1.f };
    //   }
    // }

    meshes.emplace_back(std::make_shared<Mesh>(
      name, std::move(positions), std::move(texcoords), std::move(colors),
      std::move(normals), std::move(surfaces)));
  }

  //----------------------------------------------------------------------------
  // Load nodes
  //----------------------------------------------------------------------------
  for (fg::Node& node : gltf.nodes) {
    std::shared_ptr<SceneNode> scene_node;

    if (node.meshIndex.has_value()) {
      scene_node = std::make_shared<MeshNode>();
      static_pointer_cast<MeshNode>(scene_node)->mesh = meshes[*node.meshIndex];
    }
    else {
      scene_node = std::make_shared<SceneNode>();
    }

    nodes.push_back(scene_node);
    scene.nodes[node.name.c_str()];

    std::visit(fg::visitor{
                 [&](fg::math::fmat4x4 matrix) {
                   memcpy(&scene_node->local_transform, matrix.data(),
                          sizeof(matrix));
                 },
                 [&](fg::TRS transform) {
                   Vec3f tl{ transform.translation[0], transform.translation[1],
                             transform.translation[2] };
                   Quat4f rot{ transform.rotation[3], transform.rotation[0],
                               transform.rotation[1], transform.rotation[2] };
                   Vec3f  sc{ transform.scale[0], transform.scale[1],
                             transform.scale[2] };

                   Mat4f tm{ glm::translate(glm::mat4(1.f), tl) };
                   Mat4f rm{ glm::toMat4(rot) };
                   Mat4f sm{ glm::scale(glm::mat4(1.f), sc) };

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
      scene.top_nodes.push_back(node);
      node->refreshTransform(Mat4f{ 1.f });
    }
  }
  return p_scene;
}

// TODO: support MTL file and loading texture from such a reference
// template <typename T>
// std::optional<std::shared_ptr<Scene>>
// Scene::loadObj(std::filesystem::path file_path)
// {
//   Logger log{ requestLogger("mesh") };
//   log->trace("Loading OBJ: {}", file_path.c_str());
//   // TODO: use rapidobj instead and remove tinyobjloader submodule
//   tinyobj::attrib_t                attrib;
//   std::vector<tinyobj::shape_t>    shapes;
//   std::vector<tinyobj::material_t> materials;
//   std::string                      warn, err;
//
//   // const char* env_p = std::getenv("ELDR_DIR");
//
//   if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
//                         file_path.c_str())) {
//     log->error("tinyobjloader error:\nwarn: {}\nerror: {}", warn, err);
//     return std::nullopt;
//   }
//
//   std::shared_ptr<Scene> scene{};
//
//   for (const auto& shape : shapes) {
//     std::vector<Point3f>    positions;
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
//       positions.push_back({
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
//       shape.name, std::move(positions), std::move(texcoords),
//       std::move(colors), std::move(normals), std::move(surfaces)));
//   }
//   return loaded_shapes;
// }

std::shared_ptr<Scene> Scene::load(const vk::VulkanEngine& engine,
                                   const SceneInfo&        scene_info)
{
  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }

  std::filesystem::path filepath(env_p);
  filepath /= scene_info.model_path;

  // TODO: determine file type obj/gltf if obj is to be supported too
  auto scene{
    Scene::loadGltf(engine, filepath).value_or(std::shared_ptr<Scene>{})
  };

  return scene;
}

} // namespace eldr
