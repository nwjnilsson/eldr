#pragma once
#include <app/fwd.hpp>
#include <render/fwd.hpp>
#include <vulkan/fwd.hpp>

#include <core/transform.hpp>

#include <functional>
#include <memory>
#include <vector>

NAMESPACE_BEGIN(eldr)
struct RenderObject {
  using ScalarAffineTransform4f = CoreAliases<float>::AffineTransform4f;
  uint32_t index_count;
  uint32_t first_index;
  // vk::BufferResource* index_buffer;

  const Material*         material;
  ScalarAffineTransform4f transform;
};

struct DrawContext {
  std::vector<RenderObject> opaque_surfaces;
};

struct IRenderable {
  using ScalarAffineTransform4f = CoreAliases<float>::AffineTransform4f;
  virtual void draw(const ScalarAffineTransform4f& top_matrix,
                    DrawContext&                   ctx) const = 0;
};

struct SceneNode : public IRenderable {
  virtual ~SceneNode() = default;
  // parent pointer must be a weak pointer to avoid circular dependencies
  std::weak_ptr<SceneNode>                parent;
  std::vector<std::shared_ptr<SceneNode>> children;

  ScalarAffineTransform4f local_transform;
  ScalarAffineTransform4f world_transform;

  /// @brief Apply a new transform to this node and its children
  void         refreshTransform(const ScalarAffineTransform4f& parent);
  virtual void draw(const ScalarAffineTransform4f& top_matrix,
                    DrawContext&                   ctx) const override;

  /// @brief Apply function to node and all its children recursively
  virtual void map(std::function<void(SceneNode*)> func);
};

EL_VARIANT struct MeshNode final : public SceneNode {
  std::shared_ptr<Mesh<Float, Spectrum>> mesh;
  void draw(const ScalarAffineTransform4f& top_matrix,
            DrawContext&                   ctx) const override;
};

EL_VARIANT class Scene {
  using ScalarAffineTransform4f = SceneNode::ScalarAffineTransform4f;
  friend SceneManager;
  EL_IMPORT_TYPES(Shape, Mesh)

public:
  virtual void draw(DrawContext& ctx) const;

  // std::unordered_map<std::string, std::shared_ptr<Mesh>>  meshes_;
  std::unordered_map<std::string, std::shared_ptr<Shape>> shapes_;
  // Keep nodes in scene, or move to engine?
  std::unordered_map<std::string, std::shared_ptr<SceneNode>> nodes_;
  std::vector<std::shared_ptr<SceneNode>>                     top_nodes_;

  // std::vector<Emitter> emitters_;
  // std::vector<SceneNode> scene_;
  //  std::vector<Sensor> sensors_;
};
NAMESPACE_END(eldr)
