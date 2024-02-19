#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>

namespace eldr {
namespace vk {
SwapChainSupportDetails swapChainSupportDetails(VkPhysicalDevice device,
                                                VkSurfaceKHR     surface)
{
  SwapChainSupportDetails details;
  // Query for supported formats
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);
  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count,
                                         details.formats.data());
  }

  // Query for present modes
  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_mode_count, details.present_modes.data());
  }

  return details;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
                                     VkSurfaceKHR     surface)
{
  QueueFamilyIndices indices;
  uint32_t           count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count,
                                           queue_families.data());

  // For now, only the graphics queue is found and set. Perhaps there will
  // be a need for other queues (memory or others) in the future?
  // Note on queue families: it is "very" likely that the graphics queue
  // family will be the same as the present queue family
  for (uint32_t i = 0; i < count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
    if (present_support)
      indices.present_family = i;
  }
  return indices;
}

uint32_t findMemoryType(VkPhysicalDevice device, uint32_t type_filter,
                        VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties mem_props;
  vkGetPhysicalDeviceMemoryProperties(device, &mem_props);
  for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
    if (type_filter & (1 << i) &&
        (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  ThrowVk("Failed to find suitable memory type!");
  return -1;
}

} // namespace vk
} // namespace eldr
