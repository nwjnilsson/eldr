#include <vulkan/wrappers/descriptorsetlayout.hpp>
#include <vulkan/wrappers/device.hpp>

NAMESPACE_BEGIN(eldr::vk)
EL_VK_IMPL_DEV_DEFAULTS(DescriptorSetLayout)

DescriptorSetLayout::DescriptorSetLayout(
  std::string_view                        name,
  const Device&                           device,
  std::span<VkDescriptorSetLayoutBinding> bindings,
  VkDescriptorSetLayoutCreateFlags        flags)
  : Base(name, device)
{
  const VkDescriptorSetLayoutCreateInfo layout_ci{
    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext        = {},
    .flags        = flags,
    .bindingCount = static_cast<uint32_t>(bindings.size()),
    .pBindings    = bindings.data(),
  };
  if (const VkResult result{ vkCreateDescriptorSetLayout(
        device.logical(), &layout_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create descriptor set layout! ({})", result);
}

NAMESPACE_END(eldr::vk)
