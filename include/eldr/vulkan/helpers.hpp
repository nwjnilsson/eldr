#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/swapchain.hpp>

namespace eldr {
namespace vk {

SwapchainSupportDetails swapchainSupportDetails(VkPhysicalDevice, VkSurfaceKHR);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);

uint32_t findMemoryType(VkPhysicalDevice, uint32_t type_filter,
                        VkMemoryPropertyFlags);

VkFormat findDepthFormat(VkPhysicalDevice device);

bool hasStencilComponent(VkFormat format);
} // namespace vk
} // namespace eldr
