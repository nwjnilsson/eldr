#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)
class DescriptorPool : public VkDeviceObject<VkDescriptorPool> {
  using Base = VkDeviceObject<VkDescriptorPool>;

public:
  EL_VK_IMPORT_DEFAULTS(DescriptorPool)
  DescriptorPool(std::string_view                      name,
                 const Device&                         device,
                 uint32_t                              max_sets,
                 std::span<const VkDescriptorPoolSize> pool_sizes,
                 VkDescriptorPoolCreateFlags           flags = 0);

  void reset(VkDescriptorPoolResetFlags flags = 0);
};
NAMESPACE_END(eldr::vk::wr)
