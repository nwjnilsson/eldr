#include <eldr/core/math.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/core/logger.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace eldr {

Scene::Scene(const SceneInfo& scene_info) : shapes_(loadGeometry(scene_info)) {}

Scene::~Scene()
{
  while (!shapes_.empty()) {
    delete shapes_.back();
    shapes_.pop_back();
  }
}

std::vector<Shape*> Scene::loadGeometry(const SceneInfo& scene_info)
{
  tinyobj::attrib_t                attrib;
  std::vector<tinyobj::shape_t>    shapes;
  std::vector<tinyobj::material_t> materials;
  std::string                      warn, err;

  const char* env_p = std::getenv("ELDR_DIR");

  if (!tinyobj::LoadObj(
        &attrib, &shapes, &materials, &warn, &err,
        (std::string(env_p) + "/" + scene_info.model_path).c_str())) {
    throw std::runtime_error(warn + err);
  }

  std::vector<Shape*> loaded_shapes{};

  for (const auto& shape : shapes) {
    std::vector<Vec3f> positions{};
    std::vector<Vec3f> normals{};
    std::vector<Vec2f> texcoords{};

    for (const auto& index : shape.mesh.indices) {
      positions.push_back({
        attrib.vertices[3 * index.vertex_index],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2],
      });
      normals.push_back({
        attrib.normals[3 * index.normal_index],
        attrib.normals[3 * index.normal_index + 1],
        attrib.normals[3 * index.normal_index + 2],
      });
      texcoords.push_back({
        attrib.texcoords[2 * index.texcoord_index],
        // Invert y coordinate because OBJ assumes y=0 is bottom
        // left while Vulkan says y=0 is top left
        1.f - attrib.texcoords[2 * index.texcoord_index + 1],
      });
    }
    loaded_shapes.push_back(new Mesh(positions, normals, texcoords));
  }
  return loaded_shapes;
}

} // namespace eldr
