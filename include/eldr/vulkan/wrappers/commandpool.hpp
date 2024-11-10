#pragma once
#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {
class CommandPool {
public:
  CommandPool() = delete;
  CommandPool(const Device&, const VkCommandPoolCreateFlags flags =
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  ~CommandPool();

  VkCommandPool get() const { return pool_; }

  [[nodiscard]] const CommandBuffer& requestCommandBuffer();

private:
  const Device&                               device_;
  std::string                                 name_;
  VkCommandPool                               pool_{ VK_NULL_HANDLE };
  std::vector<std::unique_ptr<CommandBuffer>> command_buffers_;
};
} // namespace eldr::vk::wr
