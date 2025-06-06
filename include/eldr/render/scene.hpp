#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

// #include <eldr/core/math.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace eldr {
struct RenderObject {
  uint32_t index_count;
  uint32_t first_index;
  // vk::BufferResource* index_buffer;

  Material* material;

  CoreAliases<Float>::Mat4f transform;
  // VkDeviceAddress vertexBufferAddress; // render graph?
};

struct DrawContext {
  std::vector<RenderObject> opaque_surfaces;
};

class Renderable {
  using Mat4f = typename CoreAliases<Float>::Mat4f;
  virtual void draw(const Mat4f& top_matrix, DrawContext& ctx) const = 0;
};

struct SceneNode : public Renderable {
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
  // inline MeshNode(std::shared_ptr<Mesh> s) : mesh(s) {};
  std::shared_ptr<Mesh> mesh;
  void draw(const Mat4f& top_matrix, DrawContext& ctx) const override;
};

struct Scene : public Renderable {
  ELDR_IMPORT_CORE_TYPES()
  struct SceneInfo {
    const std::filesystem::path model_path;
  };

  virtual void draw(const Mat4f& top_matrix, DrawContext& ctx) const override;

  [[nodiscard]] static std::optional<std::shared_ptr<Scene>>
  loadGltf(const vk::VulkanEngine& engine, std::filesystem::path file_path);

  [[nodiscard]]
  static std::optional<std::shared_ptr<Scene>>
  loadObj(std::filesystem::path file_path);

  [[nodiscard]]
  static std::optional<std::shared_ptr<Scene>>
  load(const vk::VulkanEngine& engine, const SceneInfo&);

  std::unordered_map<std::string, std::shared_ptr<Mesh>>      meshes;
  std::unordered_map<std::string, std::shared_ptr<Material>>  materials;
  std::unordered_map<std::string, std::shared_ptr<SceneNode>> nodes;

  std::vector<std::shared_ptr<SceneNode>> top_nodes;

  std::shared_ptr<vk::SceneData> vk_scene_data;

  // std::vector<Emitter> emitters_;
  // std::vector<SceneNode> scene_;
  //  std::vector<Sensor> sensors_;
};
} // namespace eldr
