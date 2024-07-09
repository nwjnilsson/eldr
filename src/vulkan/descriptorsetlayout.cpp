#include <eldr/core/logger.hpp>
#include <eldr/vulkan/descriptorsetlayout.hpp>

namespace eldr {
namespace vk {

DescriptorSetLayout::DescriptorSetLayout(const Device* device) : device_(device)
{
  VkDescriptorSetLayoutBinding ubo_layout_binding{};
  ubo_layout_binding.binding            = 0;
  ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.descriptorCount    = 1;
  ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
  ubo_layout_binding.pImmutableSamplers = nullptr; // optional

  VkDescriptorSetLayoutBinding sampler_layout_binding{};
  sampler_layout_binding.binding         = 1;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.descriptorType =
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.pImmutableSamplers = nullptr;
  sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings{
    ubo_layout_binding, sampler_layout_binding
  };

  VkDescriptorSetLayoutCreateInfo layout_ci{};
  layout_ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_ci.pBindings    = bindings.data();

  if (vkCreateDescriptorSetLayout(device_->logical(), &layout_ci, nullptr,
                                  &layout_) != VK_SUCCESS)
    ThrowVk("Failed to create descriptor set layout!");
}
DescriptorSetLayout::~DescriptorSetLayout()
{
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyDescriptorSetLayout(device_->logical(), layout_, nullptr);
}
} // namespace vk
} // namespace eldr
