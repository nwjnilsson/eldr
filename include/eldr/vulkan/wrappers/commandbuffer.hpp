#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>

namespace eldr::vk::wr {

class CommandBuffer {
public:
  CommandBuffer();
  CommandBuffer(const Device& device_, const CommandPool&);
  ~CommandBuffer();

  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer& operator=(CommandBuffer&&)      = delete;

  const VkCommandBuffer& get() const { return command_buffer_; }
  void                   begin(VkCommandBufferUsageFlags usage = 0) const;
  void                   beginSingleCommand() const;
  void                   submit() const;
  void beginRenderPass(VkRenderPass, VkFramebuffer, VkExtent2D,
                       uint32_t clear_value_count, VkClearValue*) const;
  void endRenderPass() const;
  void end() const;
  void reset() const;

private:
  const Device&      device_;
  const CommandPool& command_pool_;

  VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
