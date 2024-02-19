#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>

namespace eldr {
namespace vk {

class DescriptorSetLayout {
public:
  DescriptorSetLayout(const Device*);
  ~DescriptorSetLayout();

  const VkDescriptorSetLayout& get() const { return layout_; }
  VkDescriptorSetLayout& get() { return layout_; }

private:
  const Device* device_;

  VkDescriptorSetLayout layout_;
};
} // namespace vk
} // namespace eldr
