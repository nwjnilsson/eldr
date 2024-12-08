#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/render/fwd.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

// fwd
namespace eldr::vk {
class VulkanEngine;
}

namespace eldr {

struct RenderObject {
  uint32_t index_count;
  uint32_t first_index;
  // vk::BufferResource* index_buffer;

  MaterialType material;

  glm::mat4 transform;
  // VkDeviceAddress vertexBufferAddress; // render graph?
};

struct DrawContext {
  std::vector<RenderObject> opaque_surfaces;
};

class IRenderable {
  using Mat4f = typename CoreAliases<Float>::Mat4f;
  virtual void draw(const Mat4f& top_matrix, DrawContext& ctx) const = 0;
};

struct SceneNode : public IRenderable {
  using Mat4f = typename CoreAliases<Float>::Mat4f;

  virtual ~SceneNode() = default;
  // parent pointer must be a weak pointer to avoid circular dependencies
  std::weak_ptr<SceneNode>                parent;
  std::vector<std::shared_ptr<SceneNode>> children;

  Mat4f local_transform;
  Mat4f world_transform;

  void         refreshTransform(const Mat4f& parent_matrix);
  virtual void draw(const Mat4f& top_matrix, DrawContext& ctx) const override;

  /// @brief Apply function to node and all its children recursively
  virtual void map(std::function<void(SceneNode*)> func);
};

struct MeshNode final : public SceneNode {
  inline MeshNode(std::shared_ptr<Mesh> s) : mesh(s) {};
  std::shared_ptr<Mesh> mesh;
  void draw(const Mat4f& top_matrix, DrawContext& ctx) const override;
};

class Scene {
  ELDR_IMPORT_CORE_TYPES();
  struct SceneInfo {
    const std::filesystem::path model_path;
  };

public:
  Scene()  = default;
  ~Scene() = default;

  //[[nodiscard]] std::vector<Shape*> shapes() const { return shapes_; }
  //[[nodiscard]] const std::vector<SceneNode>& nodes() const { return nodes_; }

  static void loadGeometry(vk::VulkanEngine* engine, const SceneInfo&);

private:
  // std::vector<Emitter> emitters_;
  // std::vector<SceneNode> nodes_;
  // std::vector<SceneNode> scene_;
  //  std::vector<Sensor> sensors_;
};
} // namespace eldr
