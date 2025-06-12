#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
//------------------------------------------------------------------------------
// DescriptorSetLayoutImpl
//------------------------------------------------------------------------------
class DescriptorSetLayout::DescriptorSetLayoutImpl {
public:
  DescriptorSetLayoutImpl(const Device&                          device,
                          const VkDescriptorSetLayoutCreateInfo& layout_ci);
  ~DescriptorSetLayoutImpl();
  const Device&         device_;
  VkDescriptorSetLayout layout_{ VK_NULL_HANDLE };
};

DescriptorSetLayout::DescriptorSetLayoutImpl::DescriptorSetLayoutImpl(
  const Device& device, const VkDescriptorSetLayoutCreateInfo& layout_ci)
  : device_(device)
{

  if (const VkResult result{ vkCreateDescriptorSetLayout(
        device_.logical(), &layout_ci, nullptr, &layout_) };
      result != VK_SUCCESS)
    Throw("Failed to create descriptor set layout! ({})", result);
}

DescriptorSetLayout::DescriptorSetLayoutImpl::~DescriptorSetLayoutImpl()
{
  vkDestroyDescriptorSetLayout(device_.logical(), layout_, nullptr);
}

//------------------------------------------------------------------------------
// DescriptorSetLayout
//------------------------------------------------------------------------------
DescriptorSetLayout::DescriptorSetLayout() = default;
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&&) noexcept =
  default;
DescriptorSetLayout::~DescriptorSetLayout() = default;
DescriptorSetLayout&
DescriptorSetLayout::operator=(DescriptorSetLayout&&) = default;

DescriptorSetLayout::DescriptorSetLayout(
  const Device&                           device,
  std::span<VkDescriptorSetLayoutBinding> bindings,
  VkDescriptorSetLayoutCreateFlags        flags)
{
  const VkDescriptorSetLayoutCreateInfo layout_ci{
    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext        = {},
    .flags        = flags,
    .bindingCount = static_cast<uint32_t>(bindings.size()),
    .pBindings    = bindings.data(),
  };
  d_ = std::make_unique<DescriptorSetLayoutImpl>(device, layout_ci);
}

VkDescriptorSetLayout DescriptorSetLayout::vk() const { return d_->layout_; }
NAMESPACE_END(eldr::vk::wr)
