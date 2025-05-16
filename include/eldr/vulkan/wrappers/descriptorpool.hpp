#pragma once

#include <eldr/vulkan/common.hpp>

#include <span>

namespace eldr::vk::wr {
class DescriptorPool {
public:
  DescriptorPool() = default;
  DescriptorPool(const Device& device, uint32_t max_sets,
                 std::span<const VkDescriptorPoolSize> pool_sizes,
                 VkDescriptorPoolCreateFlags           flags = 0);

  [[nodiscard]] VkDescriptorPool get() const;
  void                           reset(VkDescriptorPoolResetFlags flags = 0);

private:
  class DescriptorPoolImpl;
  std::shared_ptr<DescriptorPoolImpl> dp_data_;
};
} // namespace eldr::vk::wr
