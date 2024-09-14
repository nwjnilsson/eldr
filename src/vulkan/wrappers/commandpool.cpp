#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>
#include <eldr/vulkan/helpers.hpp>

namespace eldr::vk::wr {
CommandPool::CommandPool(Device& device, Surface& surface)
  : device_(device)
{
  QueueFamilyIndices queue_family_indices =
    findQueueFamilies(device_.physical(), surface.get());

  VkCommandPoolCreateInfo pool_ci{};
  pool_ci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_ci.queueFamilyIndex = queue_family_indices.graphics_family.value();
  if (vkCreateCommandPool(device_.logical(), &pool_ci, nullptr, &pool_) !=
      VK_SUCCESS)
    ThrowVk("Failed to create command pool!");
}
CommandPool::~CommandPool()
{
  vkDestroyCommandPool(device_.logical(), pool_, nullptr);
}
} // namespace vk
