#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk {

void DescriptorWriter::reset()
{
  buffer_infos_.clear();
  image_infos_.clear();
  write_sets_.clear();
}

DescriptorWriter& DescriptorWriter::writeImage(uint32_t         binding,
                                               VkImageView      image,
                                               VkSampler        sampler,
                                               VkDescriptorType type,
                                               VkImageLayout    layout)
{
  image_infos_.push_back({
    .sampler     = sampler,
    .imageView   = image,
    .imageLayout = layout,
  });

  write_sets_.push_back({
    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .pNext            = {},
    .dstSet           = nullptr,
    .dstBinding       = binding,
    .dstArrayElement  = 0,
    .descriptorCount  = 1,
    .descriptorType   = type,
    .pImageInfo       = &image_infos_.back(),
    .pBufferInfo      = {},
    .pTexelBufferView = {},
  });

  return *this;
}

DescriptorWriter& DescriptorWriter::writeSampler(uint32_t           binding,
                                                 const wr::Sampler& sampler)
{
  return writeImage(
    binding, VK_NULL_HANDLE, sampler.vk(), VK_DESCRIPTOR_TYPE_SAMPLER, {});
}

DescriptorWriter& DescriptorWriter::writeSampledImage(
  uint32_t binding, const wr::ImageView& image, VkImageLayout layout)
{
  return writeImage(binding,
                    image.vk(),
                    VK_NULL_HANDLE,
                    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    layout);
}

DescriptorWriter&
DescriptorWriter::writeCombinedImageSampler(uint32_t           binding,
                                            const wr::Image&   image,
                                            const wr::Sampler& sampler,
                                            VkImageLayout      layout)
{
  return writeImage(binding,
                    image.view().vk(),
                    sampler.vk(),
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    layout);
}

DescriptorWriter& DescriptorWriter::writeStorageImage(
  uint32_t binding, const wr::ImageView& image, VkImageLayout layout)
{
  return writeImage(binding,
                    image.vk(),
                    VK_NULL_HANDLE,
                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    layout);
}

void DescriptorWriter::updateSet(const wr::Device& device, VkDescriptorSet set)
{
  for (auto& write : write_sets_)
    write.dstSet = set;

  vkUpdateDescriptorSets(device.logical(),
                         static_cast<uint32_t>(write_sets_.size()),
                         write_sets_.data(),
                         0,
                         nullptr);
}
} // namespace eldr::vk
