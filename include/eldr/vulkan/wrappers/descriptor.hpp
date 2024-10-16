#pragma once

#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class ResourceDescriptor {
public:
  ResourceDescriptor(const Device&, std::vector<VkDescriptorSetLayoutBinding>&&,
                     std::vector<VkWriteDescriptorSet>&&);
  ~ResourceDescriptor();

  // const VkDescriptorSet& get() const { return descriptor_set_; }

private:
  const Device& device_;

  VkDescriptorPool      descriptor_pool_{ VK_NULL_HANDLE };
  VkDescriptorSetLayout descriptor_set_layout_{ VK_NULL_HANDLE };
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
  std::vector<VkDescriptorSet>              descriptor_sets_;
};
} // namespace eldr::vk::wr
