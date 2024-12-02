#include <eldr/core/exceptions.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <fmt/format.h>

namespace eldr {

Scene::Scene(const SceneInfo& scene_info) : shapes_(loadGeometry(scene_info)) {}

std::vector<std::shared_ptr<Shape>>
Scene::loadGeometry(const SceneInfo& scene_info)
{

  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }
  // TODO: build a vector of texture filenames from some configuration and load
  // each texture
  std::filesystem::path filepath(env_p);
  filepath /= scene_info.model_path;
  return Mesh::loadObjMeshes(filepath).value_or(
    std::vector<std::shared_ptr<Shape>>{});
}

} // namespace eldr
