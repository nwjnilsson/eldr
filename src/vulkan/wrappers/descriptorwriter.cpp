#include <eldr/vulkan/wrappers/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
DescriptorWriter::DescriptorWriter(const Device& device, std::string_view name)
  : device_(device), name_(name)
{
  // Create layout
  //  const VkDescriptorSetLayoutCreateInfo layout_ci{
  //    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
  //    .pNext        = {},
  //    .flags        = {},
  //    .bindingCount = static_cast<uint32_t>(bindings_.size()),
  //    .pBindings    = bindings_.data(),
  //  };
  //
  //  if (const VkResult result = vkCreateDescriptorSetLayout(
  //        device_.logical(), &layout_ci, nullptr, &descriptor_set_layout_);
  //      result != VK_SUCCESS)
  //    ThrowVk(result, "vkCreateDescriptorSetLayout(): ");

  // updateSet(descriptor_sets_[0]);
  // for (size_t i = 0; i < write_sets_.size(); ++i) {
  //   write_sets_[i].dstSet = descriptor_sets_[0];
  //   // write_sets_[i].dstBinding = static_cast<uint32_t>(i);
  // }
  // vkUpdateDescriptorSets(device_.logical(),
  //                        static_cast<uint32_t>(write_sets_.size()),
  //                        write_sets_.data(), 0, nullptr);
}

DescriptorWriter::DescriptorWriter(DescriptorWriter&& other) noexcept
  : device_(other.device_), name_(std::move(other.name_))
{
  // name_ = std::exchange(other.name_, "");
  //   descriptor_set_layout_ =
  //     std::exchange(other.descriptor_set_layout_, VK_NULL_HANDLE);
}

DescriptorWriter::~DescriptorWriter()
{
  //  if (descriptor_set_layout_ != VK_NULL_HANDLE)
  //    vkDestroyDescriptorSetLayout(device_.logical(), descriptor_set_layout_,
  //                                 nullptr);
}

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
