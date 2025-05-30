#pragma once
#include <eldr/vulkan/vulkan.hpp>

namespace eldr::vk::wr {

class Fence {
public:
  Fence() = default;
  Fence(const Device& device);

  [[nodiscard]] VkFence vk() const;

  VkResult reset() const;
  VkResult wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
  [[nodiscard]] VkResult status() const;

private:
  class FenceImpl;
  std::shared_ptr<FenceImpl> d_;
};
} // namespace eldr::vk::wr
