#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace eldr {
// fwd
class EldrApp;
class Window;
namespace vk {

class VulkanEngine {
  ELDR_IMPORT_CORE_TYPES();
  friend EldrApp;

public:
  VulkanEngine() = delete;
  VulkanEngine(const Window& window);
  ~VulkanEngine();

  void updateImGui(std::function<void()> const& lambda);

  void submitGeometry(const std::vector<Vec3f>& positions,
                      const std::vector<Vec2f>& texcoords);
  void drawFrame();

  [[nodiscard]] std::string deviceName() const;

private:
  void loadTextures();
  void loadShaders();
  void createUniformBuffers();
  void createCommandBuffers();
  void createResourceDescriptors();
  void setupRenderGraph();
  void recreateSwapchain();
  void updateUniformBuffer(uint32_t current_image);
  void record(uint32_t image_index);

private:
  const Window& window_;
  core::Logger  log_{ core::requestLogger("vulkan-engine") };

  bool     initialized_{ false };
  bool     swapchain_invalidated_{ false };
  uint32_t current_frame_{ 0 };

  std::unique_ptr<wr::Instance>            instance_;
  std::unique_ptr<wr::DebugUtilsMessenger> debug_messenger_;
  std::unique_ptr<wr::Surface>             surface_;
  std::unique_ptr<wr::Device>              device_;
  std::unique_ptr<wr::Swapchain>           swapchain_;
  std::vector<wr::ResourceDescriptor>      descriptors_;
  std::unique_ptr<RenderGraph>             render_graph_;
  std::unique_ptr<ImGuiOverlay>            imgui_overlay_;
  std::vector<wr::GpuTexture>              textures_;
  std::vector<GpuVertex>                   vertices_;
  std::vector<uint32_t>                    indices_;
  std::vector<wr::Shader>                  shaders_;
  TextureResource*                         back_buffer_{ nullptr };
  TextureResource*                         msaa_buffer_{ nullptr };
  BufferResource*                          vertex_buffer_{ nullptr };
  BufferResource*                          index_buffer_{ nullptr };
  std::vector<wr::GpuBuffer>               uniform_buffers_;
  std::vector<const wr::CommandBuffer*>    in_flight_cmd_bufs_;
  std::unique_ptr<wr::GpuTexture>          viking_texture_;
};

} // namespace vk
} // namespace eldr
