#pragma once

#include <eldr/vulkan/common.hpp>

#include <optional>
#include <vector>

namespace eldr::vk::wr {
struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   present_modes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool                    isComplete()
  {
    return graphics_family.has_value() && present_family.has_value();
  }
};

class Device {
public:
  Device(Instance&, Surface&, std::vector<const char*>& required_extensions);

  ~Device();

  void                  waitIdle() const;
  VkSampleCountFlagBits getMaxMSAASampleCount() const;
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling                tiling,
                               VkFormatFeatureFlags         features) const;
  VkFormat findDepthFormat() const;
  uint32_t findMemoryType(uint32_t              type_filter,
                          VkMemoryPropertyFlags properties) const;

  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspect_flags,
                              uint32_t           mip_levels);

  // Accessors
  VkPhysicalDevice physical() const { return physical_device_; }
  VkDevice         logical() const { return device_; }
  VkQueue          graphicsQueue() const { return g_queue_; }
  VkQueue          presentQueue() const { return p_queue_; }

private:
  VkPhysicalDevice physical_device_{ VK_NULL_HANDLE };
  VkDevice         device_{ VK_NULL_HANDLE };
  VkQueue          p_queue_{ VK_NULL_HANDLE }; // present
  VkQueue          g_queue_{ VK_NULL_HANDLE }; // graphics
};
QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
SwapchainSupportDetails getSwapchainSupportDetails(VkPhysicalDevice,
                                                   VkSurfaceKHR);
} // namespace eldr::vk::wr
