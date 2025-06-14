#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <functional>

// fwd declarations
NAMESPACE_BEGIN(eldr)
class EldrApp;
NAMESPACE_END(eldr)
NAMESPACE_BEGIN(fastgltf)
struct Asset;
}

NAMESPACE_BEGIN(eldr::vk)
class VulkanEngine {
  EL_IMPORT_CORE_TYPES_SCALAR()
  friend EldrApp; // TODO: I did this to be able to invalidate swapchain from
                  // EldrApp. This is probably not ideal and there should be a
                  // better way to do this

public:
  VulkanEngine() = delete;
  VulkanEngine(const Window& window);
  ~VulkanEngine();

  void updateImGui(std::function<void()> const& lambda);

  // drawFrame should perhaps take a vector of Shapes as argument once all
  // meshes/shapes are registered in the engine
  EL_VARIANT void drawFrame(const Scene<Float, Spectrum>* scene);

  [[nodiscard]] std::string deviceName() const;
  [[nodiscard]] std::unique_ptr<SceneResources>
  createResources(fastgltf::Asset&) const;

  void buildMaterialPipelines(GltfMetallicRoughness& material);

  // TODO: decide how to handle this
  void invalidateSwapchain() { swapchain_invalidated_ = true; }

private:
  void            loadTextures();
  void            loadShaders();
  void            setupFrameData();
  void            initDescriptors();
  void            initDefaultData();
  void            createCommandBuffers();
  void            setupRenderGraph();
  void            recreateSwapchain();
  EL_VARIANT void updateBuffers(const Scene<Float, Spectrum>*);
  void            updateScene(uint32_t current_image);
  void            drawGeometry(const wr::CommandBuffer& cb);

private:
  const Window& window_;

  bool     initialized_{ false };
  bool     swapchain_invalidated_{ false };
  uint32_t frame_index_{ 0 };

  DrawContext main_draw_context_;

  struct EngineData;
  std::unique_ptr<EngineData> d_;
  struct Settings;
  std::unique_ptr<Settings> s_;

  // std::unordered_map<Mesh*, GpuMeshBuffers> mesh_buffer_table_;
};

NAMESPACE_END(eldr::vk)
template <> struct std::hash<eldr::vk::GpuVertex> {
  using Float = float;
  EL_IMPORT_CORE_TYPES()
  size_t operator()(eldr::vk::GpuVertex const& vertex) const;
};
