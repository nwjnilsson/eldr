#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr {

class IRenderable {
  ELDR_IMPORT_CORE_TYPES();
  virtual void draw(const Mat4f& top_matrix, vk::DrawContext& ctx) = 0;
};

struct SceneNode : public IRenderable {
  ELDR_IMPORT_CORE_TYPES();
  // parent pointer must be a weak pointer to avoid circular dependencies
  std::weak_ptr<SceneNode>                parent;
  std::vector<std::shared_ptr<SceneNode>> children;

  Mat4f local_transform;
  Mat4f world_transform;

  void         refreshTransform(const Mat4f& parent_matrix);
  virtual void draw(const Mat4f& top_matrix, vk::DrawContext& ctx) override;
};

struct MeshNode : public SceneNode {
  std::shared_ptr<Mesh> mesh;
  virtual void draw(const Mat4f& top_matrix, vk::DrawContext& ctx) override;
};
} // namespace eldr
