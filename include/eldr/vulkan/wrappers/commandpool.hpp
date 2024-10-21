#pragma once
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {
class CommandPool {
public:
  CommandPool() = delete;
  CommandPool(const Device&, const VkCommandPoolCreateFlags flags =
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  ~CommandPool();

  VkCommandPool get() const { return pool_; }

private:
  const Device& device_;

  VkCommandPool pool_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
