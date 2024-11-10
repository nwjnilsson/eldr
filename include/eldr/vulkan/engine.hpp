#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/imgui.hpp>

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

  void initImGui();
  void shutdownImGui();

  void drawFrame();
  void submitGeometry(const std::vector<Vec3f>& positions,
                      const std::vector<Vec2f>& texcoords);

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
  const Window&                   window_;
  std::shared_ptr<spdlog::logger> log_{ detail::requestLogger(
    "vulkan-engine") };

  bool                  initialized_{ false };
  bool                  framebuffer_resized_{ false };
  uint32_t              current_frame_{ 0 };
  VkSampleCountFlagBits msaa_sample_count_{ VK_SAMPLE_COUNT_1_BIT };

  std::unique_ptr<wr::DebugUtilsMessenger> debug_messenger_;
  std::unique_ptr<wr::Instance>            instance_;
  std::unique_ptr<wr::Surface>             surface_;
  std::unique_ptr<wr::Device>              device_;
  std::unique_ptr<wr::Swapchain>           swapchain_;
  std::vector<wr::ResourceDescriptor>      descriptors_;
  std::unique_ptr<RenderGraph>             render_graph_;
  std::unique_ptr<ImGuiOverlay>            imgui_overlay_;
  // std::unique_ptr<wr::Pipeline>       pipeline_;
  // std::unique_ptr<wr::CommandPool> command_pool_;
  std::vector<wr::GpuTexture> textures_;
  // std::unique_ptr<wr::Buffer>      vertex_buffer_;
  // std::unique_ptr<wr::Buffer>      index_buffer_;
  std::vector<GpuVertex>     vertices_;
  std::vector<uint32_t>      indices_;
  std::vector<wr::Shader>    shaders_;
  TextureResource*           back_buffer_;
  BufferResource*            vertex_buffer_{ nullptr };
  BufferResource*            index_buffer_{ nullptr };
  std::vector<wr::GpuBuffer> uniform_buffers_;
  std::vector<void*>         uniform_buffers_mapped_;
  // std::vector<wr::CommandBuffer>      command_buffers_;
  std::vector<const wr::CommandBuffer*> in_flight_cmd_bufs_;
  std::unique_ptr<wr::GpuTexture>       viking_texture_;
  // std::vector<wr::Semaphore>            image_available_sem_;
  // std::vector<wr::Semaphore>            render_finished_sem_;
};

} // namespace vk
} // namespace eldr
