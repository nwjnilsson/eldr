#pragma once

#include <eldr/vulkan/buffer.hpp>
#include <eldr/vulkan/commandpool.hpp>
#include <eldr/vulkan/descriptorpool.hpp>
#include <eldr/vulkan/descriptorsetlayout.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/fence.hpp>
#include <eldr/vulkan/instance.hpp>
#include <eldr/vulkan/pipeline.hpp>
#include <eldr/vulkan/sampler.hpp>
#include <eldr/vulkan/semaphore.hpp>
#include <eldr/vulkan/swapchain.hpp>
#include <eldr/vulkan/texturesampler.hpp>

namespace eldr {

namespace vk {

class VulkanWrapper {
public:
  VulkanWrapper();
  VulkanWrapper(GLFWwindow* const,
                std::vector<const char*>& instance_extensions);
  ~VulkanWrapper();

  void drawFrame();

private:
  void createUniformBuffers();
  void createCommandBuffers();
  void createDescriptorSets();
  void updateUniformBuffer(uint32_t current_image);
  void record(uint32_t image_index);

private:
  GLFWwindow* const window_;

  uint32_t current_frame_;

  Instance                     instance_;
  VkDebugUtilsMessengerEXT     debug_messenger_;
  Surface                      surface_;
  Device                       device_;
  Swapchain                    swapchain_;
  RenderPass                   render_pass_;
  DescriptorSetLayout          descriptor_set_layout_;
  DescriptorPool               descriptor_pool_;
  Pipeline                     pipeline_;
  CommandPool                  command_pool_;
  TextureSampler               texture_sampler_;
  Buffer                       vertex_buffer_;
  Buffer                       index_buffer_;
  std::vector<Buffer>          uniform_buffers_;
  std::vector<void*>           uniform_buffers_mapped_;
  std::vector<VkCommandBuffer> command_buffers_;
  std::vector<VkDescriptorSet> descriptor_sets_;
  std::vector<Semaphore>       image_available_sem_;
  std::vector<Semaphore>       render_finished_sem_;
  std::vector<Fence>           in_flight_fences_;
};
} // namespace vk
} // namespace eldr
