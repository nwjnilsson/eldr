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

  void waitIdle() const;

  [[nodiscard]] VkSampleCountFlagBits maxMSAASampleCount() const;
  [[nodiscard]] VkFormat
  findSupportedFormat(const std::vector<VkFormat>& candidates,
                      VkImageTiling                tiling,
                      VkFormatFeatureFlags         features) const;

  [[nodiscard]] VkFormat findDepthFormat() const;
  [[nodiscard]] uint32_t findMemoryType(uint32_t              type_filter,
                                        VkMemoryPropertyFlags properties) const;

  // Accessors
  [[nodiscard]] const QueueFamilyIndices& queueFamilyIndices() const;
  [[nodiscard]] VkPhysicalDevice physical() const { return physical_device_; }
  [[nodiscard]] VkDevice         logical() const { return device_; }
  [[nodiscard]] VkQueue          graphicsQueue() const { return g_queue_; }
  [[nodiscard]] VkQueue          presentQueue() const { return p_queue_; }
  [[nodiscard]] VmaAllocator     allocator() const { return allocator_; }

private:
  VkPhysicalDevice   physical_device_{ VK_NULL_HANDLE };
  VkDevice           device_{ VK_NULL_HANDLE };
  VmaAllocator       allocator_{ VK_NULL_HANDLE };
  QueueFamilyIndices queue_family_indices_{};
  VkQueue            p_queue_{ VK_NULL_HANDLE }; // present
  VkQueue            g_queue_{ VK_NULL_HANDLE }; // graphics
};
// QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
SwapchainSupportDetails getSwapchainSupportDetails(VkPhysicalDevice,
                                                   VkSurfaceKHR);
} // namespace eldr::vk::wr
