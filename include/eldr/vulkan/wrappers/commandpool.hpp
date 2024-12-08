#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

#include <vector>

namespace eldr::vk::wr {
class CommandPool {
public:
  CommandPool() = default;
  CommandPool(const Device&                  device,
              const VkCommandPoolCreateFlags flags =
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  [[nodiscard]] VkCommandPool get() const;

  [[nodiscard]] const CommandBuffer& requestCommandBuffer();

private:
  // std::string name_;

  class CommandPoolImpl;
  std::shared_ptr<CommandPoolImpl> cp_data_;

  std::vector<CommandBuffer> command_buffers_{};
};
} // namespace eldr::vk::wr
