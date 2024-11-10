#pragma once

#include <eldr/vulkan/common.hpp>

#include <string>
#include <vector>

namespace eldr::vk::wr {

class ResourceDescriptor {
public:
  ResourceDescriptor(const Device&,
                     std::vector<VkDescriptorSetLayoutBinding>&& bindings,
                     std::vector<VkWriteDescriptorSet>&& descriptor_writes,
                     const std::string&                  name);
  ResourceDescriptor(ResourceDescriptor&&) noexcept;
  ResourceDescriptor(const ResourceDescriptor&) = delete;
  ~ResourceDescriptor();

  [[nodiscard]] const std::vector<VkDescriptorSet>& descriptorSets() const
  {
    return descriptor_sets_;
  }

  [[nodiscard]] VkDescriptorSetLayout descriptorSetLayout() const
  {
    return descriptor_set_layout_;
  }
  // const VkDescriptorSet& get() const { return descriptor_set_; }

private:
  const Device&         device_;
  std::string           name_;
  VkDescriptorPool      descriptor_pool_{ VK_NULL_HANDLE };
  VkDescriptorSetLayout descriptor_set_layout_{ VK_NULL_HANDLE };
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
  std::vector<VkWriteDescriptorSet>         descriptor_writes_;
  std::vector<VkDescriptorSet>              descriptor_sets_;
};
} // namespace eldr::vk::wr
