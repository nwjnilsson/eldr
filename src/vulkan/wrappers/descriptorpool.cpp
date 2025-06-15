#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(DescriptorPool)
EL_VK_IMPL_DESTRUCTOR(DescriptorPool)

DescriptorPool::DescriptorPool(std::string_view                      name,
                               const Device&                         device,
                               uint32_t                              max_sets,
                               std::span<const VkDescriptorPoolSize> pool_sizes,
                               VkDescriptorPoolCreateFlags           flags)
  : Base(name, device)
{
  const VkDescriptorPoolCreateInfo pool_ci{
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = {},
    .flags         = flags,
    .maxSets       = max_sets,
    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
    .pPoolSizes    = pool_sizes.data(),
  };
  if (const VkResult result{
        vkCreateDescriptorPool(device.logical(), &pool_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create descriptor pool! ({})", result);
}

void DescriptorPool::reset(VkDescriptorPoolResetFlags flags)
{
  vkResetDescriptorPool(device().logical(), object_, flags);
}
NAMESPACE_END(eldr::vk::wr)
