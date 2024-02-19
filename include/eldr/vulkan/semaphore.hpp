#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>

namespace eldr {
namespace vk {

class Semaphore {
public:
  Semaphore();
  Semaphore(const Device* device_);
  Semaphore(Semaphore&&);
  ~Semaphore();

  const VkSemaphore& get() const { return semaphore_; }
  VkSemaphore& get() { return semaphore_; }

private:
  const Device* device_;

  VkSemaphore semaphore_;
};
} // namespace vk
} // namespace eldr
