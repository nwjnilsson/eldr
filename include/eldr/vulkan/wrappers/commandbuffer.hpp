#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>

namespace eldr::vk::wr {

class CommandBuffer {
public:
  CommandBuffer();
  CommandBuffer(Device& device_, CommandPool&);
  ~CommandBuffer();

  CommandBuffer& operator=(CommandBuffer&&);

  VkCommandBuffer& get() { return command_buffer_; }
  void             begin(VkCommandBufferUsageFlags usage = 0);
  void             beginSingleCommand();
  void             submit();
  void             beginRenderPass(VkRenderPass, VkFramebuffer, VkExtent2D,
                                   uint32_t clear_value_count, VkClearValue*);
  void             endRenderPass();
  void             end();
  void             reset();

private:
  Device&      device_;
  CommandPool& command_pool_;

  VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
