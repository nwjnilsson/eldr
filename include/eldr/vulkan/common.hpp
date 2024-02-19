#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#  define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>

#include <optional>
#include <vector>

namespace eldr {
namespace vk {
constexpr uint8_t max_frames_in_flight = 2;

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool                    isComplete()
  {
    return graphics_family.has_value() && present_family.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   present_modes;
};

} // namespace vk
} // namespace eldr
