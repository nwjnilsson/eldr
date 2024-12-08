#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// DescriptorPoolImpl
//------------------------------------------------------------------------------
class DescriptorPool::DescriptorPoolImpl {
public:
  DescriptorPoolImpl(const Device&                     device,
                     const VkDescriptorPoolCreateInfo& ci);
  ~DescriptorPoolImpl();
  const Device     device_;
  VkDescriptorPool pool_{ VK_NULL_HANDLE };
};

DescriptorPool::DescriptorPoolImpl::DescriptorPoolImpl(
  const Device& device, const VkDescriptorPoolCreateInfo& pool_ci)
  : device_(device)
{

  if (const auto result =
        vkCreateDescriptorPool(device_.logical(), &pool_ci, nullptr, &pool_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorPool(): ");
}

DescriptorPool::DescriptorPoolImpl::~DescriptorPoolImpl()
{
  vkDestroyDescriptorPool(device_.logical(), pool_, nullptr);
}

//------------------------------------------------------------------------------
// DescriptorPool
//------------------------------------------------------------------------------
DescriptorPool::DescriptorPool(const Device& device, uint32_t max_sets,
                               std::span<VkDescriptorPoolSize> pool_sizes,
                               VkDescriptorPoolCreateFlags     flags)
{
  const VkDescriptorPoolCreateInfo pool_ci{
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = {},
    .flags         = flags,
    .maxSets       = max_sets,
    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
    .pPoolSizes    = pool_sizes.data(),
  };
  std::make_shared<DescriptorPoolImpl>(device, pool_ci);
}

VkDescriptorPool DescriptorPool::get() const { return dp_data_->pool_; }

void DescriptorPool::reset(VkDescriptorPoolResetFlags flags)
{
  vkResetDescriptorPool(dp_data_->device_.logical(), dp_data_->pool_, flags);
}
} // namespace eldr::vk::wr
