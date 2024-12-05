#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

DescriptorSetLayout::DescriptorSetLayout(
  const Device& device, std::span<VkDescriptorSetLayoutBinding> bindings,
  VkDescriptorSetLayoutCreateFlags flags)
  : device_(device)
{
  const VkDescriptorSetLayoutCreateInfo layout_ci{
    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext        = {},
    .flags        = flags,
    .bindingCount = static_cast<uint32_t>(bindings.size()),
    .pBindings    = bindings.data(),
  };

  if (const auto result = vkCreateDescriptorSetLayout(
        device_.logical(), &layout_ci, nullptr, &layout_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorSetLayout(): ");
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
  : device_(other.device_)
{
  layout_ = std::exchange(other.layout_, VK_NULL_HANDLE);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(device_.logical(), layout_, nullptr);
}
} // namespace eldr::vk::wr
