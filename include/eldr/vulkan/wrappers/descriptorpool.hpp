#pragma once

#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class DescriptorPool {
public:
  DescriptorPool(const Device&, const std::vector<VkDescriptorPoolSize>&,
                 VkDescriptorPoolCreateFlags flags, uint32_t max_sets);
  ~DescriptorPool();

  VkDescriptorPool get() const { return descriptor_pool_; }

private:
  const Device& device_;

  VkDescriptorPool descriptor_pool_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
