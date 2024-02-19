#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>

namespace eldr {
namespace vk {
class CommandPool {
public:
  CommandPool();
  CommandPool(const Device*, const Surface&);
  ~CommandPool();

  const VkCommandPool& get() const { return pool_; }
  VkCommandPool& get() { return pool_; }

private:
  const Device* device_;

  VkCommandPool pool_;
};
} // namespace vk
} // namespace eldr
