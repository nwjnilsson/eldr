#pragma once
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk::wr)
class CommandPool {
public:
  CommandPool();
  CommandPool(const Device&                  device,
              const VkCommandPoolCreateFlags flags =
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  CommandPool(CommandPool&&) noexcept;
  ~CommandPool();

  [[nodiscard]] VkCommandPool vk() const;

  [[nodiscard]] const CommandBuffer& requestCommandBuffer();

private:
  // std::string name_;

  class CommandPoolImpl;
  std::unique_ptr<CommandPoolImpl> d_;

  std::vector<CommandBuffer> command_buffers_;
};
NAMESPACE_END(eldr::vk::wr)
