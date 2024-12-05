#pragma once
#include <eldr/vulkan/common.hpp>

#include <span>

namespace eldr::vk::wr {

class DescriptorSetLayout {
public:
  DescriptorSetLayout()                           = delete;
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout(DescriptorSetLayout&&) noexcept;
  DescriptorSetLayout(const Device&, std::span<VkDescriptorSetLayoutBinding>,
                      VkDescriptorSetLayoutCreateFlags flags);
  ~DescriptorSetLayout();

  [[nodiscard]] VkDescriptorSetLayout get() const { return layout_; }

private:
  const Device& device_;

  VkDescriptorSetLayout layout_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
