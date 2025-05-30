#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

namespace eldr::vk::wr {
class DescriptorPool {
public:
  DescriptorPool() = default;
  DescriptorPool(const Device&                         device,
                 uint32_t                              max_sets,
                 std::span<const VkDescriptorPoolSize> pool_sizes,
                 VkDescriptorPoolCreateFlags           flags = 0);

  [[nodiscard]] VkDescriptorPool vk() const;
  void                           reset(VkDescriptorPoolResetFlags flags = 0);

private:
  class DescriptorPoolImpl;
  std::shared_ptr<DescriptorPoolImpl> d_;
};
} // namespace eldr::vk::wr
