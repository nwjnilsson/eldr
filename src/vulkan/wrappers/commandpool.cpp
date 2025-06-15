#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(CommandPool)
CommandPool::~CommandPool()
{
  if (vk()) {
    command_buffers_.clear();
    vkDestroyCommandPool(device().logical(), object_, nullptr);
  }
}

CommandPool::CommandPool(std::string_view               name,
                         const Device&                  device,
                         const VkCommandPoolCreateFlags flags)
  : Base(name, device)
{
  const VkCommandPoolCreateInfo pool_ci{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = {},
    .flags            = flags,
    .queueFamilyIndex = device.queueFamilyIndices().graphics_family.value(),
  };

  if (const VkResult result{
        vkCreateCommandPool(device.logical(), &pool_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create command pool ({})", result);
}

const CommandBuffer& CommandPool::requestCommandBuffer()
{
  // Try to find a command buffer that is not in use
  for (const auto& cb : command_buffers_) {
    if (cb.fenceStatus() == VK_SUCCESS) {
      // Reset the command buffer's fence to make it usable again
      cb.resetFence();
      // The command buffer is reset implicitly since the command pools are
      // created with the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag
      // (default value in CommandPool constructor). Without this flag, the
      // explicit reset below is necessary.
      // cb->reset();
      cb.begin();
      return cb;
    }
  }

  // We need to create a new command buffer because no free one was found
  // Note that there is currently no method for shrinking command_buffers_, but
  // this should not be a problem
  const std::string name{ fmt::format("command buffer #{}",
                                      command_buffers_.size() + 1) };
  Log(Trace, "Creating {}", name);
  command_buffers_.emplace_back(name, device(), *this);

  command_buffers_.back().begin();
  return command_buffers_.back();
}
NAMESPACE_END(eldr::vk::wr)
