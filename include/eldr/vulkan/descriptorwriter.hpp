#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>

#include <deque>

namespace eldr::vk {
class DescriptorWriter {
public:
  void reset();

  template <typename T>
  DescriptorWriter& writeUniformBuffer(uint32_t          binding,
                                       const wr::Buffer& buffer, size_t offset);
  template <typename T>
  DescriptorWriter& writeStorageBuffer(uint32_t          binding,
                                       const wr::Buffer& buffer, size_t offset);

  DescriptorWriter& writeSampler(uint32_t binding, const wr::Sampler& sampler);

  DescriptorWriter& writeSampledImage(uint32_t             binding,
                                      const wr::ImageView& image,
                                      VkImageLayout        layout);

  DescriptorWriter& writeCombinedImageSampler(
    uint32_t binding, const wr::GpuTexture& texture,
    VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  DescriptorWriter& writeStorageImage(uint32_t             binding,
                                      const wr::ImageView& image,
                                      VkImageLayout        layout);

  void updateSet(const wr::Device& device, VkDescriptorSet set);

private:
  template <typename T>
  DescriptorWriter& writeBuffer(uint32_t binding, const wr::Buffer& buffer,
                                size_t offset, VkDescriptorType type);
  DescriptorWriter&
  writeImage(uint32_t binding, VkImageView image, VkSampler sampler,
             VkDescriptorType type,
             VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

private:
  std::deque<VkDescriptorBufferInfo> buffer_infos_;
  std::deque<VkDescriptorImageInfo>  image_infos_;
  std::vector<VkWriteDescriptorSet>  write_sets_;
};

template <typename T>
DescriptorWriter&
DescriptorWriter::writeBuffer(uint32_t binding, const wr::Buffer& buffer,
                              VkDeviceSize offset, VkDescriptorType type)
{
  buffer_infos_.push_back({
    .buffer = buffer.get(),
    .offset = offset,
    .range  = sizeof(T),
  });

  write_sets_.push_back(VkWriteDescriptorSet{
    .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .pNext            = {},
    .dstSet           = nullptr,
    .dstBinding       = binding,
    .dstArrayElement  = 0,
    .descriptorCount  = 1,
    .descriptorType   = type,
    .pImageInfo       = {},
    .pBufferInfo      = &buffer_infos_.back(),
    .pTexelBufferView = {},
  });

  return *this;
}

template <typename T>
DescriptorWriter& DescriptorWriter::writeUniformBuffer(uint32_t binding,
                                                       const wr::Buffer& buffer,
                                                       size_t            offset)
{
  return writeBuffer<T>(binding, buffer, offset,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

template <typename T>
DescriptorWriter& DescriptorWriter::writeStorageBuffer(uint32_t binding,
                                                       const wr::Buffer& buffer,
                                                       size_t            offset)
{
  return writeBuffer<T>(binding, buffer, offset,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}
} // namespace eldr::vk