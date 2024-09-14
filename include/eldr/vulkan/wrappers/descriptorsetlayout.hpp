#pragma once

#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class DescriptorSetLayout {
public:
  DescriptorSetLayout(Device&,
                      const std::vector<VkDescriptorSetLayoutBinding>&);
  ~DescriptorSetLayout();

  const VkDescriptorSetLayout& get() const { return layout_; }

private:
  Device& device_;

  VkDescriptorSetLayout layout_;
};
} // namespace eldr::vk::wr
