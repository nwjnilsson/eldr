#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

DescriptorSetLayout::DescriptorSetLayout(
  const Device&                                    device,
  const std::vector<VkDescriptorSetLayoutBinding>& bindings)
  : device_(device)
{
  const VkDescriptorSetLayoutCreateInfo layout_ci{
    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext        = {},
    .flags        = {},
    .bindingCount = static_cast<uint32_t>(bindings.size()),
    .pBindings    = bindings.data(),
  };

  if (const auto result = vkCreateDescriptorSetLayout(
        device_.logical(), &layout_ci, nullptr, &layout_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorSetLayout(): ");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(device_.logical(), layout_, nullptr);
}
} // namespace eldr::vk::wr
