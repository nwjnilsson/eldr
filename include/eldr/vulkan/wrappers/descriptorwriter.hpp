#pragma once

#include <eldr/vulkan/common.hpp>

#include <deque>
#include <vector>

namespace eldr::vk::wr {

class DescriptorWriter {
public:
  inline DescriptorWriter(const Device& device) : device_(device) {};
  DescriptorWriter(DescriptorWriter&&) noexcept = default;
  DescriptorWriter(const DescriptorWriter&)     = delete;
  ~DescriptorWriter()                           = default;

  void clear();

  template <typename T>
  DescriptorWriter& writeBuffer(uint32_t binding, VkBuffer buffer,
                                VkDescriptorType type);
  template <typename T>
  DescriptorWriter& writeUniformBuffer(uint32_t binding, VkBuffer buffer);
  template <typename T>
  DescriptorWriter& writeStorageBuffer(uint32_t binding, VkBuffer buffer);

  DescriptorWriter&
  writeImage(uint32_t binding, VkDescriptorType type, VkImageView image,
             VkSampler     sampler,
             VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  DescriptorWriter& writeSampler(uint32_t binding, VkSampler sampler);

  DescriptorWriter& writeSampledImage(uint32_t binding, VkImageView image,
                                      VkImageLayout layout);

  DescriptorWriter& writeCombinedImageSampler(
    uint32_t binding, VkImageView image, VkSampler sampler,
    VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  DescriptorWriter& writeStorageImage(uint32_t binding, VkImageView image,
                                      VkImageLayout layout);

  void updateSet(VkDescriptorSet set);

private:
  const Device& device_;
  // std::string   name_;

  std::deque<VkDescriptorBufferInfo> buffer_infos_;
  std::deque<VkDescriptorImageInfo>  image_infos_;
  std::vector<VkWriteDescriptorSet>  write_sets_;
};

template <typename T>
DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t         binding,
                                                VkBuffer         buffer,
                                                VkDescriptorType type)
{
  assert(buffer);

  // layout_bindings_.push_back({
  //   .binding            = binding,
  //   .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
  //   .descriptorCount    = 1,
  //   .stageFlags         = static_cast<VkShaderStageFlags>(shader_stage),
  //   .pImmutableSamplers = nullptr,
  // });

  buffer_infos_.push_back({
    .buffer = buffer,
    .offset = 0,
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
                                                       VkBuffer buffer)
{
  return writeBuffer<T>(binding, buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

template <typename T>
DescriptorWriter& DescriptorWriter::writeStorageBuffer(uint32_t binding,
                                                       VkBuffer buffer)
{
  return writeBuffer<T>(binding, buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}
} // namespace eldr::vk::wr
