#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

namespace eldr::vk::wr {
CommandPool::CommandPool(const Device&                  device,
                         const VkCommandPoolCreateFlags flags)
  : device_(device)
{
  VkCommandPoolCreateInfo pool_ci{};
  pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_ci.flags = flags;
  pool_ci.queueFamilyIndex =
    device_.queueFamilyIndices().graphics_family.value();
  if (vkCreateCommandPool(device_.logical(), &pool_ci, nullptr, &pool_) !=
      VK_SUCCESS)
    ThrowVk("Failed to create command pool!");
}

CommandPool::~CommandPool()
{
  vkDestroyCommandPool(device_.logical(), pool_, nullptr);
}

const CommandBuffer& CommandPool::requestCommandBuffer() const
{
  // Try to find a command buffer which is currently not used
  for (const auto& cb : command_buffers_) {
    if (cb->fenceStatus() == VK_SUCCESS) {
      // Reset the command buffer's fence to make it usable again
      cb->resetFence();
      cb->begin();
      return *cb;
    }
  }

  // We need to create a new command buffer because no free one was found
  // Note that there is currently no method for shrinking command_buffers_, but
  // this should not be a problem
  command_buffers_.emplace_back(
    std::make_unique<CommandBuffer>(device_, *this, "command buffer"));

  spdlog::trace("Creating new command buffer #{}", command_buffers_.size());

  command_buffers_.back()->begin();
  return *command_buffers_.back();
}
} // namespace eldr::vk::wr
