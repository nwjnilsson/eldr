#pragma once
#include <eldr/math/matrix.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace eldr::render {
struct RenderObject {
  using Matrix4f = CoreAliases<float>::Matrix4f;
  uint32_t index_count;
  uint32_t first_index;
  // vk::BufferResource* index_buffer;

  Material* material;
  // Matrix4f  transform;
};

struct DrawContext {
  std::vector<RenderObject> opaque_surfaces;
};

struct Renderable {
  using Matrix4f = CoreAliases<float>::Matrix4f;
  virtual void draw(const Matrix4f& top_matrix, DrawContext& ctx) const = 0;
};

struct SceneNode : public Renderable {
  using Matrix4f = CoreAliases<float>::Matrix4f;

  virtual ~SceneNode() = default;
  // parent pointer must be a weak pointer to avoid circular dependencies
  std::weak_ptr<SceneNode>                parent;
  std::vector<std::shared_ptr<SceneNode>> children;

  Matrix4f local_transform;
  Matrix4f world_transform;

  void         refreshTransform(const Matrix4f& parent_matrix);
  virtual void draw(const Matrix4f& top_matrix,
                    DrawContext&    ctx) const override;

  /// @brief Apply function to node and all its children recursively
  virtual void map(std::function<void(SceneNode*)> func);
};

EL_VARIANT struct MeshNode final : public SceneNode {
  // inline MeshNode(std::shared_ptr<Mesh> s) : mesh(s) {};
  using Matrix4f = CoreAliases<float>::Matrix4f;
  std::shared_ptr<Mesh<Float, Spectrum>> mesh;
  void draw(const Matrix4f& top_matrix, DrawContext& ctx) const override;
};

class RenderableScene {};

EL_VARIANT class Scene : public RenderableScene {
  EL_IMPORT_TYPES(Shape, Mesh)
public:
  struct SceneInfo {
    const std::filesystem::path model_path;
  };

  [[nodiscard]] static std::optional<std::shared_ptr<Scene>>
  loadGltf(const vk::VulkanEngine& engine, std::filesystem::path file_path);

  [[nodiscard]]
  static std::optional<std::shared_ptr<Scene>>
  loadObj(std::filesystem::path file_path);

  [[nodiscard]]
  static std::optional<std::shared_ptr<Scene>>
  load(const vk::VulkanEngine& engine, const SceneInfo&);

  void draw(DrawContext& ctx) const;

  std::unordered_map<std::string, std::shared_ptr<Mesh>>  meshes_;
  std::unordered_map<std::string, std::shared_ptr<Shape>> shapes_;
  // Keep nodes in scene, or move to engine?
  std::unordered_map<std::string, std::shared_ptr<SceneNode>> nodes_;
  std::vector<std::shared_ptr<SceneNode>>                     top_nodes_;

  // TODO: keep or move everything vulkan related to engine? loading a scene can
  // "register" the scene in the engine, and in the scene's desctructor, one
  // would unregister the scene. Materials could be added in the same way, thus
  // decoupling vulkan stuff from material struct
  std::unique_ptr<vk::SceneResources> vk_resources_;

  // std::vector<Emitter> emitters_;
  // std::vector<SceneNode> scene_;
  //  std::vector<Sensor> sensors_;
};
} // namespace eldr::render
