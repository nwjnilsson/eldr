#pragma once
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

class Fence {
public:
  Fence(const Device&);
  Fence(Fence&&);
  ~Fence();

  VkFence get() const { return fence_; }

  void reset() const;
  [[nodiscard]] VkResult
  wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
  [[nodiscard]] VkResult status() const;

private:
  const Device& device_;

  VkFence fence_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
