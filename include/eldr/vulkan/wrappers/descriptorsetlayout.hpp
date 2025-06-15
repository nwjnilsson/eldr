#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)

class DescriptorSetLayout : public VkDeviceObject<VkDescriptorSetLayout> {
  using Base = VkDeviceObject<VkDescriptorSetLayout>;

public:
  EL_VK_IMPORT_DEFAULTS(DescriptorSetLayout)
  DescriptorSetLayout(std::string_view name,
                      const Device&,
                      std::span<VkDescriptorSetLayoutBinding>,
                      VkDescriptorSetLayoutCreateFlags flags);
};
NAMESPACE_END(eldr::vk::wr)
