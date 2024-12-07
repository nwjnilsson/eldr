#pragma once
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

class Fence {
public:
  Fence(const Device&);
  Fence(Fence&&);
  ~Fence();

  [[nodiscard]] VkFence get() const;

  VkResult reset();
  [[nodiscard]] VkResult
  wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
  [[nodiscard]] VkResult status() const;

private:
  class FenceImpl;
  std::shared_ptr<FenceImpl> f_data_;
};
} // namespace eldr::vk::wr
