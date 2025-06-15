#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/core/config.hpp>
#include <eldr/math/math.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

NAMESPACE_BEGIN(eldr)
struct RenderObject {
  using Matrix4f = CoreAliases<float>::Matrix4f;
  uint32_t index_count;
  uint32_t first_index;
  // vk::BufferResource* index_buffer;

  const Material* material;
  // Matrix4f  transform;
};

struct DrawContext {
  std::vector<RenderObject> opaque_surfaces;
};

struct Renderable {
  using Transform4f = CoreAliases<float>::Transform4f;
  virtual void draw(const Transform4f& top_matrix, DrawContext& ctx) const = 0;
};

struct SceneNode : public Renderable {
  using Transform4f = CoreAliases<float>::Transform4f;

  virtual ~SceneNode() = default;
  // parent pointer must be a weak pointer to avoid circular dependencies
  std::weak_ptr<SceneNode>                parent;
  std::vector<std::shared_ptr<SceneNode>> children;

  Transform4f local_transform;
  Transform4f world_transform;

  void         refreshTransform(const Transform4f& parent);
  virtual void draw(const Transform4f& top_matrix,
                    DrawContext&       ctx) const override;

  /// @brief Apply function to node and all its children recursively
  virtual void map(std::function<void(SceneNode*)> func);
};

EL_VARIANT struct MeshNode final : public SceneNode {
  using Transform4f = CoreAliases<float>::Transform4f;
  std::shared_ptr<Mesh<Float, Spectrum>> mesh;
  void draw(const Transform4f& top_matrix, DrawContext& ctx) const override;
};
EL_INSTANTIATE_STRUCT(MeshNode)

class SceneBase {

  std::string name_;
};

EL_VARIANT class Scene : public SceneBase {
  EL_IMPORT_TYPES(Shape, Mesh)
  friend SceneManager;

public:
  void draw(DrawContext& ctx) const;

  std::unordered_map<std::string, std::shared_ptr<Mesh>>  meshes_;
  std::unordered_map<std::string, std::shared_ptr<Shape>> shapes_;
  // Keep nodes in scene, or move to engine?
  std::unordered_map<std::string, std::shared_ptr<SceneNode>> nodes_;
  std::vector<std::shared_ptr<SceneNode>>                     top_nodes_;

  // std::vector<Emitter> emitters_;
  // std::vector<SceneNode> scene_;
  //  std::vector<Sensor> sensors_;
};
NAMESPACE_END(eldr)
