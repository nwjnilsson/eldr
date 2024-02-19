#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/instance.hpp>
#include <eldr/vulkan/surface.hpp>

namespace eldr {
namespace vk {
class Device {
public:
  Device(const Instance&, const Surface&);

  ~Device();

  const VkPhysicalDevice& physical() const { return physical_device_; }
  const VkDevice&         logical() const { return device_; }
  const VkQueue&          graphicsQueue() const { return g_queue_; }
  const VkQueue&          presentQueue() const { return p_queue_; }

private:
  VkPhysicalDevice physical_device_;
  VkDevice         device_;
  VkQueue          p_queue_; // present
  VkQueue          g_queue_; // graphics
};
} // namespace vk
} // namespace eldr
