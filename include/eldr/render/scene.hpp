#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <filesystem>
#include <memory>
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

struct ShapeNode final : public SceneNode {
  std::shared_ptr<Shape> shape;
  virtual void draw(const Mat4f& top_matrix, vk::DrawContext& ctx) override;
};

class Scene {
  ELDR_IMPORT_CORE_TYPES();
  struct SceneInfo {
    const std::filesystem::path model_path;
    const std::filesystem::path texture_path;
  };

public:
  Scene(const SceneInfo&);
  ~Scene() = default;

  //[[nodiscard]] std::vector<Shape*> shapes() const { return shapes_; }
  [[nodiscard]] const std::vector<std::shared_ptr<Shape>>& shapes() const
  {
    return shapes_;
  }

private:
  std::vector<std::shared_ptr<Shape>> loadGeometry(const SceneInfo&);

private:
  // std::vector<Emitter> emitters_;
  std::vector<std::shared_ptr<Shape>> shapes_;
  // std::vector<SceneNode> scene_;
  //  std::vector<Sensor> sensors_;
};
} // namespace eldr
