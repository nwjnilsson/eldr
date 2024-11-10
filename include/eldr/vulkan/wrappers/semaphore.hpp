#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

namespace eldr::vk::wr {

class Semaphore {
public:
  Semaphore(const Device& device_);
  Semaphore(Semaphore&&);
  ~Semaphore();

  VkSemaphore        get() const { return semaphore_; }
  const VkSemaphore* ptr() const { return &semaphore_; }

private:
  const Device& device_;

  VkSemaphore semaphore_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
