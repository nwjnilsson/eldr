#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

DescriptorPool::DescriptorPool(const Device& device, uint32_t max_sets,
                               std::span<VkDescriptorPoolSize> pool_sizes,
                               VkDescriptorPoolCreateFlags     flags)
  : device_(device)
{
  const VkDescriptorPoolCreateInfo pool_ci{
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = {},
    .flags         = flags,
    .maxSets       = max_sets,
    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
    .pPoolSizes    = pool_sizes.data(),
  };

  if (const auto result =
        vkCreateDescriptorPool(device_.logical(), &pool_ci, nullptr, &pool_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorPool(): ");
}

DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
  : device_(other.device_)
{
  pool_ = std::exchange(other.pool_, VK_NULL_HANDLE);
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other)
{
  assert(&device_ == &other.device_);
  pool_ = std::exchange(other.pool_, VK_NULL_HANDLE);
  return *this;
}

DescriptorPool::~DescriptorPool()
{
  if (pool_ != VK_NULL_HANDLE)
    vkDestroyDescriptorPool(device_.logical(), pool_, nullptr);
}

void DescriptorPool::reset(VkDescriptorPoolResetFlags flags)
{
  vkResetDescriptorPool(device_.logical(), pool_, flags);
}
} // namespace eldr::vk::wr
