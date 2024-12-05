#include <eldr/vulkan/wrappers/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

void DescriptorWriter::clear()
{
  buffer_infos_.clear();
  image_infos_.clear();
  write_sets_.clear();
}

DescriptorWriter& DescriptorWriter::writeImage(uint32_t         binding,
                                               VkDescriptorType type,
                                               VkImageView      image,
                                               VkSampler        sampler,
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

DescriptorWriter& DescriptorWriter::writeSampler(uint32_t  binding,
                                                 VkSampler sampler)
{
  return writeImage(binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_NULL_HANDLE,
                    sampler, {});
}

DescriptorWriter& DescriptorWriter::writeSampledImage(uint32_t      binding,
                                                      VkImageView   image,
                                                      VkImageLayout layout)
{
  return writeImage(binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, image,
                    VK_NULL_HANDLE, layout);
}

DescriptorWriter& DescriptorWriter::writeCombinedImageSampler(
  uint32_t binding, VkImageView image, VkSampler sampler, VkImageLayout layout)
{
  return writeImage(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, image,
                    sampler, layout);
}

DescriptorWriter& DescriptorWriter::writeStorageImage(uint32_t      binding,
                                                      VkImageView   image,
                                                      VkImageLayout layout)
{
  return writeImage(binding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, image,
                    VK_NULL_HANDLE, layout);
}

void DescriptorWriter::updateSet(VkDescriptorSet set)
{
  for (auto& write : write_sets_)
    write.dstSet = set;

  vkUpdateDescriptorSets(device_.logical(),
                         static_cast<uint32_t>(write_sets_.size()),
                         write_sets_.data(), 0, nullptr);
}
} // namespace eldr::vk::wr
