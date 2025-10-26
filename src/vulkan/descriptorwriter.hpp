#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/wrappers/buffer.hpp>
#include <vulkan/wrappers/image.hpp>

#include <deque>

NAMESPACE_BEGIN(eldr::vk)
class DescriptorWriter {
public:
  void reset();

  template <typename T>
  DescriptorWriter& writeUniformBuffer(uint32_t             binding,
                                       const Buffer<T>& buffer,
                                       size_t               offset = 0);
  template <typename T>
  DescriptorWriter& writeStorageBuffer(uint32_t             binding,
                                       const Buffer<T>& buffer,
                                       size_t               offset = 0);

  DescriptorWriter& writeSampler(uint32_t binding, const Sampler& sampler);

  DescriptorWriter& writeSampledImage(uint32_t             binding,
                                      const ImageView& image,
                                      VkImageLayout        layout);

  DescriptorWriter& writeCombinedImageSampler(
    uint32_t           binding,
    const Image&   image,
    const Sampler& sampler,
    VkImageLayout      layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  DescriptorWriter& writeStorageImage(uint32_t             binding,
                                      const ImageView& image,
                                      VkImageLayout        layout);

  void updateSet(const Device& device, VkDescriptorSet set);

private:
  template <typename T>
  DescriptorWriter& writeBuffer(uint32_t             binding,
                                const Buffer<T>& buffer,
                                size_t               offset,
                                VkDescriptorType     type);
  DescriptorWriter&
  writeImage(uint32_t         binding,
             VkImageView      image,
             VkSampler        sampler,
             VkDescriptorType type,
             VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

private:
  std::deque<VkDescriptorBufferInfo> buffer_infos_;
  std::deque<VkDescriptorImageInfo>  image_infos_;
  std::vector<VkWriteDescriptorSet>  write_sets_;
};

template <typename T>
DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t             binding,
                                                const Buffer<T>& buffer,
                                                size_t           index_offset,
                                                VkDescriptorType type)
{
  buffer_infos_.push_back({
    .buffer = buffer.vk(),
    .offset = index_offset * sizeof(T),
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
DescriptorWriter& DescriptorWriter::writeUniformBuffer(
  uint32_t binding, const Buffer<T>& buffer, size_t index_offset)
{
  return writeBuffer<T>(
    binding, buffer, index_offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

template <typename T>
DescriptorWriter& DescriptorWriter::writeStorageBuffer(
  uint32_t binding, const Buffer<T>& buffer, size_t offset)
{
  return writeBuffer<T>(
    binding, buffer, offset, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}
NAMESPACE_END(eldr::vk)
