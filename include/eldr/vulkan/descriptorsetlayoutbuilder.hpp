#pragma once
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk)
class DescriptorSetLayoutBuilder {
public:
  void                        reset() { bindings_.clear(); }
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

  [[nodiscard]] wr::DescriptorSetLayout
  build(const wr::Device&                device,
        VkDescriptorSetLayoutCreateFlags create_flags = 0);

private:
  DescriptorSetLayoutBuilder&
  add(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage_flags);

private:
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
};
NAMESPACE_END(eldr::vk)
