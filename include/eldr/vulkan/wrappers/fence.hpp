#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Fence {
public:
  Fence();
  Fence(const Device& device);
  Fence(Fence&&) noexcept = default;
  ~Fence();

  [[nodiscard]] VkFence vk() const;

  VkResult reset() const;
  VkResult wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
  [[nodiscard]] VkResult status() const;

private:
  class FenceImpl;
  std::unique_ptr<FenceImpl> d_;
};
NAMESPACE_END(eldr::vk::wr)
