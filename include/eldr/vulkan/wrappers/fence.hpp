#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

class Fence {
public:
  Fence(const Device&);
  Fence(Fence&&);
  ~Fence();

  VkFence get() const { return fence_; }
  void    reset();
  void    wait();

private:
  const Device& device_;

  VkFence fence_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
