#pragma once

#include <eldr/vulkan/common.hpp>

#include <span>

namespace eldr::vk::wr {

class DescriptorPool {
public:
  DescriptorPool()                      = delete;
  DescriptorPool(const DescriptorPool&) = delete;
  DescriptorPool(DescriptorPool&&) noexcept;
  DescriptorPool(const Device& device, uint32_t max_sets,
                 std::span<VkDescriptorPoolSize> pool_sizes,
                 VkDescriptorPoolCreateFlags     flags = 0);
  ~DescriptorPool();

  DescriptorPool& operator=(DescriptorPool&& other);

  VkDescriptorPool get() const { return pool_; }
  void             reset(VkDescriptorPoolResetFlags flags = 0);

private:
  const Device& device_;

  VkDescriptorPool pool_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
