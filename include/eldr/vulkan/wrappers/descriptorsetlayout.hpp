#pragma once
#include <eldr/vulkan/common.hpp>

#include <span>

namespace eldr::vk::wr {

class DescriptorSetLayout {
public:
  DescriptorSetLayout(const Device&, std::span<VkDescriptorSetLayoutBinding>,
                      VkDescriptorSetLayoutCreateFlags flags);

  [[nodiscard]] VkDescriptorSetLayout get() const;

private:
  class DescriptorSetLayoutImpl;
  std::shared_ptr<DescriptorSetLayoutImpl> dsl_data_;
};
} // namespace eldr::vk::wr
