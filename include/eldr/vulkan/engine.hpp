#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <functional>

// fwd declarations
NAMESPACE_BEGIN(fastgltf)
struct Asset;
NAMESPACE_END(fastgltf)

NAMESPACE_BEGIN(eldr::vk)
class VulkanEngine {
  using Float    = float;
  using Spectrum = Color<float, 4>;
  EL_IMPORT_TYPES(Shape, Scene, MeshNode)

public:
  VulkanEngine() = delete;
  VulkanEngine(const Window& window);
  ~VulkanEngine();

  void updateImGui(std::function<void()> const& lambda);

  // drawFrame should perhaps take a vector of Shapes as argument once all
  // meshes/shapes are registered in the engine
  void drawFrame(const Scene* scene);

  [[nodiscard]] std::string deviceName() const;
  [[nodiscard]] std::vector<const Material*>
  loadMaterials(fastgltf::Asset&) const;

  void buildMaterialPipelines(GltfMetallicRoughness& material);

  // TODO: decide how to handle this
  void invalidateSwapchain() { swapchain_invalidated_ = true; }

  void setCamera(const Camera& camera) { camera_ = &camera; }

private:
  void loadTextures();
  void loadShaders();
  void setupFrameData();
  void initDescriptors();
  void initDefaultData();
  void createCommandBuffers();
  void setupRenderGraph();
  void recreateSwapchain();
  void updateBuffers(const Scene*);
  void updateScene(const Scene*);
  void drawGeometry(const CommandBuffer& cb);

private:
  const Window& window_;
  const Camera* camera_;

  bool     initialized_{ false };
  bool     swapchain_invalidated_{ false };
  uint32_t frame_index_{ 0 };

  DrawContext main_draw_context_;

  struct EngineData;
  std::unique_ptr<EngineData> d_;
  struct Settings;
  std::unique_ptr<Settings> s_;
};

NAMESPACE_END(eldr::vk)
template <> struct std::hash<eldr::vk::GpuVertex> {
  EL_IMPORT_CORE_TYPES_SCALAR()
  size_t operator()(eldr::vk::GpuVertex const& vertex) const;
};
