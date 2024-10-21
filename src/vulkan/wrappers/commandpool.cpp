#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>
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
  if (vkCreateCommandPool(device_.logical(), &pool_ci, nullptr, &pool_) !=
      VK_SUCCESS)
    ThrowVk("Failed to create command pool!");
}

CommandPool::~CommandPool()
{
  vkDestroyCommandPool(device_.logical(), pool_, nullptr);
}
} // namespace eldr::vk::wr
