#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

namespace eldr::vk::wr {

class DescriptorSetLayout {
public:
  DescriptorSetLayout() = default;
  DescriptorSetLayout(const Device&,
                      std::span<VkDescriptorSetLayoutBinding>,
                      VkDescriptorSetLayoutCreateFlags flags);

  [[nodiscard]] VkDescriptorSetLayout vk() const;

private:
  class DescriptorSetLayoutImpl;
  std::shared_ptr<DescriptorSetLayoutImpl> d_;
};
} // namespace eldr::vk::wr
