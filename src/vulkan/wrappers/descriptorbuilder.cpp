#include <eldr/vulkan/wrappers/descriptorbuilder.hpp>
#include <eldr/vulkan/wrappers/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

DescriptorBuilder::DescriptorBuilder(const Device& device) : device_(device) {}

DescriptorWriter DescriptorBuilder::build(std::string_view name)
{
  assert(!layout_bindings_.empty());

  // Generate a new resource descriptor.
  DescriptorWriter generated_descriptor(device_, std::move(layout_bindings_),
                                        std::move(write_sets_), name);

  layout_bindings_.clear();
  write_sets_.clear();
  buffer_infos_.clear();
  image_infos_.clear();

  return generated_descriptor;
}

DescriptorBuilder& DescriptorBuilder::addCombinedImageSampler(
  VkSampler image_sampler, VkImageView image_view, std::uint32_t binding,
  const VkShaderStageFlagBits shader_stage)
{
  assert(image_sampler);
  assert(image_view);

  layout_bindings_.push_back({
    .binding            = binding,
    .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount    = 1,
    .stageFlags         = static_cast<VkShaderStageFlags>(shader_stage),
    .pImmutableSamplers = {},
  });

  image_infos_.push_back({
    .sampler     = image_sampler,
    .imageView   = image_view,
    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  });

  write_sets_.push_back(VkWriteDescriptorSet{
    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .pNext            = {},
    .dstSet           = nullptr,
    .dstBinding       = binding,
    .dstArrayElement  = 0,
    .descriptorCount  = 1,
    .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .pImageInfo       = &image_infos_.back(),
    .pBufferInfo      = {},
    .pTexelBufferView = {},
  });

  return *this;
}

} // namespace eldr::vk::wr
