#pragma once
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk::wr)
class CommandPool : public VkDeviceObject<VkCommandPool> {
  using Base = VkDeviceObject<VkCommandPool>;

public:
  EL_VK_IMPORT_DEFAULTS(CommandPool)
  CommandPool(std::string_view               name,
              const Device&                  device,
              const VkCommandPoolCreateFlags flags =
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  [[nodiscard]] const CommandBuffer& requestCommandBuffer();

private:
  std::vector<CommandBuffer> command_buffers_;
};
NAMESPACE_END(eldr::vk::wr)
