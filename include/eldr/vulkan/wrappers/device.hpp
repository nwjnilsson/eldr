#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <functional>
#include <optional>
#include <vector>

NAMESPACE_BEGIN(eldr::vk::wr)
struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR>   present_modes;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool                    isComplete() const
  {
    return graphics_family.has_value() && present_family.has_value();
  }
};

class Device {
public:
  Device();
  Device(const Instance&,
         const Surface&,
         const std::vector<const char*>& device_extensions);
  ~Device();

  Device& operator=(Device&&);

  [[nodiscard]] std::string name() const
  {
    return physical_device_props_.deviceName;
  }

  [[nodiscard]] VkFormat
  findSupportedFormat(const std::vector<VkFormat>& candidates,
                      VkImageTiling                tiling,
                      VkFormatFeatureFlags         features) const;

  [[nodiscard]] VkFormat findDepthFormat() const;
  [[nodiscard]] uint32_t findMemoryType(uint32_t              type_filter,
                                        VkMemoryPropertyFlags properties) const;

  [[nodiscard]] const CommandBuffer& requestCommandBuffer() const;

  // Accessors
  [[nodiscard]] VkSampleCountFlagBits     findMaxMsaaSampleCount() const;
  [[nodiscard]] const QueueFamilyIndices& queueFamilyIndices() const
  {
    return queue_family_indices_;
  };
  [[nodiscard]] SwapchainSupportDetails
  swapchainSupportDetails(VkSurfaceKHR surface) const;
  [[nodiscard]] VkPhysicalDevice physical() const;
  [[nodiscard]] VkDevice         logical() const;
  [[nodiscard]] VmaAllocator     allocator() const;
  [[nodiscard]] VkQueue          graphicsQueue() const { return g_queue_; }
  [[nodiscard]] VkQueue          presentQueue() const { return p_queue_; }

  void waitIdle() const;

  /// @brief Executes a lambda function immediately and waits.
  void execute(
    const std::function<void(const CommandBuffer& cmd_buf)>& cmd_lambda) const;

private:
  CommandPool& threadGraphicsPool() const;

private:
  class DeviceImpl;
  std::unique_ptr<DeviceImpl> d_;
  VkPhysicalDeviceProperties  physical_device_props_;
  QueueFamilyIndices          queue_family_indices_;
  VkQueue                     p_queue_{ VK_NULL_HANDLE }; // present
  VkQueue                     g_queue_{ VK_NULL_HANDLE }; // graphics
};
// QueueFamilyIndices      findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
NAMESPACE_END(eldr::vk::wr)
