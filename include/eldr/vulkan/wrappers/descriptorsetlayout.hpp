#pragma once

#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class DescriptorSetLayout {
public:
  DescriptorSetLayout(const Device&,
                      const std::vector<VkDescriptorSetLayoutBinding>&);
  ~DescriptorSetLayout();

  const VkDescriptorSetLayout& get() const { return layout_; }

private:
  const Device& device_;

  VkDescriptorSetLayout layout_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
