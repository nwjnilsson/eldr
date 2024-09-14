#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

DescriptorSetLayout::DescriptorSetLayout(
  Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
  : device_(device)
{
  auto layout_ci         = makeInfo<VkDescriptorSetLayoutCreateInfo>();
  layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_ci.pBindings    = bindings.data();

  VkResult result = vkCreateDescriptorSetLayout(device_.logical(), &layout_ci,
                                                nullptr, &layout_);
  CheckVkResult(result);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(device_.logical(), layout_, nullptr);
}
} // namespace eldr::vk::wr
