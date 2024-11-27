#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

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
  if (const auto result =
        vkCreateCommandPool(device_.logical(), &pool_ci, nullptr, &pool_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateCommandPool(): ");
}

CommandPool::~CommandPool()
{
  command_buffers_.clear();
  vkDestroyCommandPool(device_.logical(), pool_, nullptr);
}

const CommandBuffer& CommandPool::requestCommandBuffer()
{
  // Try to find a command buffer that is not in use
  for (const auto& cb : command_buffers_) {
    if (cb->fenceStatus() == VK_SUCCESS) {
      // Reset the command buffer's fence to make it usable again
      cb->resetFence();
      // The command buffer is reset implicitly since the command pools are
      // created with the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag
      // (default value in CommandPool constructor). Without this flag, the
      // explicit reset below is necessary.
      // cb->reset();
      cb->begin();
      return *cb;
    }
  }

  // We need to create a new command buffer because no free one was found
  // Note that there is currently no method for shrinking command_buffers_, but
  // this should not be a problem
  std::string name =
    fmt::format("command buffer #{}", command_buffers_.size() + 1);
  device_.logger()->trace("Creating {}", name);
  command_buffers_.emplace_back(
    std::make_unique<CommandBuffer>(device_, *this, name));

  command_buffers_.back()->begin();
  return *command_buffers_.back();
}
} // namespace eldr::vk::wr
