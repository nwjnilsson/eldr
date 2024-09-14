#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <memory>
#include <vector>

namespace eldr::vk {

class VulkanEngine {
  ELDR_IMPORT_CORE_TYPES();

public:
  VulkanEngine() = delete;
  VulkanEngine(uint32_t width, uint32_t height);
  ~VulkanEngine();

  void initImGui();
  void shutdownImGui();

  void newFrame();
  void drawFrame();
  void submitGeometry(const std::vector<Vec3f>& positions,
                      const std::vector<Vec2f>& texcoords);

private:
  bool isFrameBufferResized();
  void createUniformBuffers();
  void createCommandBuffers();
  void createResourceDescriptors();
  void updateUniformBuffer(uint32_t current_image);
  void record(uint32_t image_index);

private:
  std::unique_ptr<wr::Window> window_;

  bool     initialized_{ false };
  uint32_t current_frame_{ 0 };

  VkDebugUtilsMessengerEXT debug_messenger_;

  std::unique_ptr<wr::Instance>       instance_;
  std::unique_ptr<wr::Surface>        surface_;
  std::unique_ptr<wr::Device>         device_;
  std::unique_ptr<wr::Swapchain>      swapchain_;
  std::vector<wr::ResourceDescriptor> descriptors_;
  std::unique_ptr<wr::Pipeline>       pipeline_;
  std::unique_ptr<wr::CommandPool>    command_pool_;
  std::unique_ptr<wr::TextureSampler> texture_sampler_;
  std::unique_ptr<wr::Buffer>         vertex_buffer_;
  std::unique_ptr<wr::Buffer>         index_buffer_;
  std::vector<Vertex>                 vertices_;
  std::vector<uint32_t>               indices_;
  std::vector<wr::Buffer>             uniform_buffers_;
  std::vector<void*>                  uniform_buffers_mapped_;
  std::vector<wr::CommandBuffer>      command_buffers_;
  std::vector<wr::Semaphore>          image_available_sem_;
  std::vector<wr::Semaphore>          render_finished_sem_;
  std::vector<wr::Fence>              in_flight_fences_;
  // TODO: add frame/window structs
};

} // namespace eldr::vk
