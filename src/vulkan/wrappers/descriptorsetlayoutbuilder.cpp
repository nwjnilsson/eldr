#include <eldr/vulkan/wrappers/descriptorsetlayoutbuilder.hpp>

namespace eldr::vk::wr {
DescriptorSetLayoutBuilder&
DescriptorSetLayoutBuilder::add(uint32_t binding, VkDescriptorType type,
                                VkShaderStageFlags stage_flags)
{
  bindings_.push_back({
    .binding            = binding,
    .descriptorType     = type,
    .descriptorCount    = 1,
    .stageFlags         = stage_flags,
    .pImmutableSamplers = nullptr,
  });
  return *this;
}

DescriptorSetLayoutBuilder&
DescriptorSetLayoutBuilder::addSampler(uint32_t           binding,
                                       VkShaderStageFlags stage_flags)
{
  return add(binding, VK_DESCRIPTOR_TYPE_SAMPLER, stage_flags);
}

DescriptorSetLayoutBuilder&
DescriptorSetLayoutBuilder::addSampledImage(uint32_t           binding,
                                            VkShaderStageFlags stage_flags)
{
  return add(binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, stage_flags);
}
DescriptorSetLayoutBuilder&
DescriptorSetLayoutBuilder::addStorageImage(uint32_t           binding,
                                            VkShaderStageFlags stage_flags)
{
  return add(binding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stage_flags);
}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::addCombinedImageSampler(
  uint32_t binding, VkShaderStageFlags stage_flags)
{
  return add(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stage_flags);
}

DescriptorSetLayoutBuilder&
DescriptorSetLayoutBuilder::addUniformBuffer(uint32_t           binding,
                                             VkShaderStageFlags stage_flags)
{
  return add(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage_flags);
}

DescriptorSetLayoutBuilder&
DescriptorSetLayoutBuilder::addStorageBuffer(uint32_t           binding,
                                             VkShaderStageFlags stage_flags)
{
  return add(binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stage_flags);
}

DescriptorSetLayout
DescriptorSetLayoutBuilder::build(const Device&                    device,
                                  VkDescriptorSetLayoutCreateFlags create_flags)
{
  DescriptorSetLayout layout(device, bindings_, create_flags);
  bindings_.clear();
  return layout;
}

} // namespace eldr::vk::wr
