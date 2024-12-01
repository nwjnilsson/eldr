#include <eldr/core/logger.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/engine.hpp>

// TODO: use rapidobj
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <spdlog/spdlog.h>
namespace eldr {

Mesh::Mesh(const std::string& name, std::vector<Point3f>&& positions,
           std::vector<Point2f>&& texcoords, std::vector<Color4f>&& colors,
           std::vector<Vec3f>&& normals, std::vector<GeoSurface>&& surfaces)
  : name_(name), vtx_positions_(positions), vtx_texcoords_(texcoords),
    vtx_colors_(colors), vtx_normals_(normals), surfaces_(surfaces)

{
}

std::optional<std::vector<std::shared_ptr<Mesh>>>
Mesh::loadGltfMeshes(std::filesystem::path file_path)
{
  namespace fg = fastgltf;
  Logger log{ requestLogger("mesh") };
  log->trace("Loading GLTF: {}", file_path.c_str());

  auto data{ fg::GltfDataBuffer::FromPath(file_path) };
  if (data.error() != fg::Error::None) {
    log->error("fg error: {}", fg::getErrorName(data.error()));
    return std::nullopt;
  }

  constexpr auto gltf_options{ fg::Options::LoadExternalBuffers };

  fg::Asset  gltf;
  fg::Parser parser{};

  fg::Category category{ fg::Category::All };
  auto load{ parser.loadGltf(data.get(), file_path.parent_path(), gltf_options,
                             category) };
  if (load.error() == fg::Error::None) {
    gltf = std::move(load.get());
  }
  else {
    log->error("fg error: {}", fg::getErrorName(load.error()));
    return std::nullopt;
  }

  std::vector<std::shared_ptr<Mesh>> meshes;

  // use the same vectors for all meshes so that the memory doesnt reallocate as
  // often
  std::vector<uint32_t>   indices;
  std::vector<Point3f>    positions;
  std::vector<Point2f>    texcoords;
  std::vector<Color4f>    colors;
  std::vector<Vec3f>      normals;
  std::vector<GeoSurface> surfaces;

  for (fg::Mesh& mesh : gltf.meshes) {
    const std::string name{ mesh.name };
    indices.clear();
    positions.clear();
    texcoords.clear();
    colors.clear();
    normals.clear();

    for (auto&& p : mesh.primitives) {
      GeoSurface new_surface;
      new_surface.start_index = static_cast<uint32_t>(indices.size());
      new_surface.count =
        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count);

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
        positions.reserve(new_size);
        positions.reserve(new_size);

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
      surfaces.push_back(new_surface);
    }

    // display the vertex normals
    constexpr bool override_colors = true;
    if (override_colors) {
      for (size_t i = 0; i < colors.size(); ++i) {
        colors[i] = Color4f{ normals[i], 1.f };
      }
    }

    meshes.emplace_back(std::make_unique<Mesh>(
      name, std::move(positions), std::move(texcoords), std::move(colors),
      std::move(normals), std::move(surfaces)));
  }

  return meshes;
}

// template <typename T>
std::optional<std::vector<std::shared_ptr<Shape>>>
Mesh::loadObj(std::filesystem::path file_path)
{
  Logger log{ requestLogger("mesh") };
  log->trace("Loading OBJ: {}", file_path.c_str());
  // TODO: use rapidobj instead and remove tinyobjloader submodule
  tinyobj::attrib_t                attrib;
  std::vector<tinyobj::shape_t>    shapes;
  std::vector<tinyobj::material_t> materials;
  std::string                      warn, err;

  // const char* env_p = std::getenv("ELDR_DIR");

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        file_path.c_str())) {
    log->error("tinyobjloader error:\nwarn: {}\nerror: {}", warn, err);
    return std::nullopt;
  }

  std::vector<std::shared_ptr<Shape>> loaded_shapes{};

  for (const auto& shape : shapes) {
    std::vector<uint32_t>   indices;
    std::vector<Point3f>    positions;
    std::vector<Point2f>    texcoords;
    std::vector<Color4f>    colors;
    std::vector<Vec3f>      normals;
    std::vector<GeoSurface> surfaces;

    for (const auto& index : shape.mesh.indices) {
      positions.push_back({
        attrib.vertices[3 * index.vertex_index],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2],
      });
      texcoords.push_back({
        attrib.texcoords[2 * index.texcoord_index],
        // Invert y coordinate because OBJ assumes y=0 is bottom
        // left while Vulkan says y=0 is top left
        1.f - attrib.texcoords[2 * index.texcoord_index + 1],
      });
      normals.push_back({
        attrib.normals[3 * index.normal_index],
        attrib.normals[3 * index.normal_index + 1],
        attrib.normals[3 * index.normal_index + 2],
      });
      colors.push_back({ attrib.colors[3 * index.vertex_index],
                         attrib.colors[3 * index.vertex_index + 1],
                         attrib.colors[3 * index.vertex_index + 2], 1.0f });
    }

    log->trace("Loaded mesh '{}' - {} vertices", shape.name, positions.size());

    loaded_shapes.emplace_back(std::make_shared<Mesh>(
      shape.name, std::move(positions), std::move(texcoords), std::move(colors),
      std::move(normals), std::move(surfaces)));
  }
  return loaded_shapes;
}
} // namespace eldr
