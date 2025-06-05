#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

namespace eldr::vk::wr {

class DescriptorSetLayout {
public:
  DescriptorSetLayout();
  DescriptorSetLayout(const Device&,
                      std::span<VkDescriptorSetLayoutBinding>,
                      VkDescriptorSetLayoutCreateFlags flags);
  DescriptorSetLayout(DescriptorSetLayout&&) noexcept;
  ~DescriptorSetLayout();

  DescriptorSetLayout& operator=(DescriptorSetLayout&&);

  [[nodiscard]] VkDescriptorSetLayout vk() const;

private:
  class DescriptorSetLayoutImpl;
  std::unique_ptr<DescriptorSetLayoutImpl> d_;
};
} // namespace eldr::vk::wr
