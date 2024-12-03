#include <eldr/core/exceptions.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/engine.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

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
// Shape node
//------------------------------------------------------------------------------
void MeshNode::draw(const Mat4f& top_matrix, DrawContext& ctx) const
{
  const Mat4f node_matrix{ top_matrix * world_transform };

  for (auto& s : mesh->surfaces()) {
    const RenderObject obj{
      .index_count = s.count,
      .first_index = s.start_index,
      .material    = s.material,
      .transform   = node_matrix,
    };
    ctx.opaque_surfaces.push_back(obj);
  }
  // Draw recursively
  SceneNode::draw(top_matrix, ctx);
}

// Scene::Scene(const SceneInfo& scene_info) : nodes_(loadGeometry(scene_info))
// {}

void Scene::loadGeometry(vk::VulkanEngine* engine, const SceneInfo& scene_info)
{

  const char* env_p = std::getenv("ELDR_DIR");
  if (env_p == nullptr) {
    Throw("Environment not set up correctly");
  }
  // TODO: build a vector of texture filenames from some configuration and load
  // each texture
  std::filesystem::path filepath(env_p);
  filepath /= scene_info.model_path;
  std::vector<SceneNode> scene_nodes;
  auto                   meshes{ Mesh::loadObjMeshes(filepath).value_or(
    std::vector<std::shared_ptr<Mesh>>{}) };

  // TODO: deal with it
  if (meshes.empty())
    Throw("Error loading meshes");

  for (std::shared_ptr<Mesh>& ptr : meshes) {
    engine->addSceneNode(std::make_shared<MeshNode>(ptr));
  }
}

} // namespace eldr
