#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>

namespace eldr::vk::wr {
class DescriptorSetLayoutBuilder {
public:
  DescriptorSetLayoutBuilder& addSampler(uint32_t           binding,
                                         VkShaderStageFlags stage_flags);
  DescriptorSetLayoutBuilder& addSampledImage(uint32_t           binding,
                                              VkShaderStageFlags stage_flags);
  DescriptorSetLayoutBuilder& addStorageImage(uint32_t           binding,
                                              VkShaderStageFlags stage_flags);
  DescriptorSetLayoutBuilder&
  addCombinedImageSampler(uint32_t binding, VkShaderStageFlags stage_flags);

  DescriptorSetLayoutBuilder& addUniformBuffer(uint32_t           binding,
                                               VkShaderStageFlags stage_flags);
  DescriptorSetLayoutBuilder& addStorageBuffer(uint32_t           binding,
                                               VkShaderStageFlags stage_flags);

  [[nodiscard]] DescriptorSetLayout
  build(const Device& device, VkDescriptorSetLayoutCreateFlags create_flags);

private:
  DescriptorSetLayoutBuilder& add(uint32_t binding, VkDescriptorType type,
                                  VkShaderStageFlags stage_flags);

private:
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
};
} // namespace eldr::vk::wr
