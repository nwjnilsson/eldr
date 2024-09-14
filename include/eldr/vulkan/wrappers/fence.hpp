#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

class Fence {
public:
  Fence(Device&);
  Fence(Fence&&);
  ~Fence();

  VkFence get() const { return fence_; }
  void    reset();
  void    wait();

private:
  Device& device_;

  VkFence fence_;
};
} // namespace eldr::vk::wr
