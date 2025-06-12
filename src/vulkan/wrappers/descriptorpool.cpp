#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

//------------------------------------------------------------------------------
// DescriptorPoolImpl
//------------------------------------------------------------------------------
class DescriptorPool::DescriptorPoolImpl {
public:
  DescriptorPoolImpl(const Device&                     device,
                     const VkDescriptorPoolCreateInfo& ci);
  ~DescriptorPoolImpl();
  const Device&    device_;
  VkDescriptorPool pool_{ VK_NULL_HANDLE };
};

DescriptorPool::DescriptorPoolImpl::DescriptorPoolImpl(
  const Device& device, const VkDescriptorPoolCreateInfo& pool_ci)
  : device_(device)
{

  if (const VkResult result{
        vkCreateDescriptorPool(device_.logical(), &pool_ci, nullptr, &pool_) };
      result != VK_SUCCESS)
    Throw("Failed to create descriptor pool! ({})", result);
}

DescriptorPool::DescriptorPoolImpl::~DescriptorPoolImpl()
{
  vkDestroyDescriptorPool(device_.logical(), pool_, nullptr);
}

//------------------------------------------------------------------------------
// DescriptorPool
//------------------------------------------------------------------------------
DescriptorPool::DescriptorPool()  = default;
DescriptorPool::~DescriptorPool() = default;
DescriptorPool::DescriptorPool(const Device&                         device,
                               uint32_t                              max_sets,
                               std::span<const VkDescriptorPoolSize> pool_sizes,
                               VkDescriptorPoolCreateFlags           flags)
{
  const VkDescriptorPoolCreateInfo pool_ci{
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = {},
    .flags         = flags,
    .maxSets       = max_sets,
    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
    .pPoolSizes    = pool_sizes.data(),
  };
  d_ = std::make_unique<DescriptorPoolImpl>(device, pool_ci);
}
DescriptorPool::DescriptorPool(DescriptorPool&&) noexcept   = default;
DescriptorPool& DescriptorPool::operator=(DescriptorPool&&) = default;

VkDescriptorPool DescriptorPool::vk() const { return d_->pool_; }

void DescriptorPool::reset(VkDescriptorPoolResetFlags flags)
{
  vkResetDescriptorPool(d_->device_.logical(), d_->pool_, flags);
}
NAMESPACE_END(eldr::vk::wr)
