#pragma once

#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class DescriptorPool {
public:
  DescriptorPool(Device&, const std::vector<VkDescriptorPoolSize>&,
                 VkDescriptorPoolCreateFlags flags, uint32_t max_sets);
  ~DescriptorPool();

  VkDescriptorPool get() const { return descriptor_pool_; }

private:
  Device& device_;

  VkDescriptorPool descriptor_pool_;
};
} // namespace eldr::vk::wr
