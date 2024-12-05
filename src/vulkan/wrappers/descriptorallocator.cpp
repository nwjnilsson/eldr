#include <eldr/vulkan/wrappers/descriptorallocator.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

DescriptorAllocator::DescriptorAllocator(const Device&            device,
                                         uint32_t                 max_sets,
                                         std::span<PoolSizeRatio> ratios)
  : device_(device), ratios_(ratios.begin(), ratios.end()),
    sets_per_pool_(std::min(max_sets, max_sets_limit))
{
  ready_pools_.push_back(createPool(ratios_));
}

DescriptorAllocator::~DescriptorAllocator() { destroyPools(); }

VkDescriptorPool DescriptorAllocator::getPool()
{
  VkDescriptorPool new_pool;
  if (!ready_pools_.empty()) {
    new_pool = ready_pools_.back();
    ready_pools_.pop_back();
  }
  else {
    // need to create a new pool
    sets_per_pool_ =
      std::min(static_cast<uint32_t>(sets_per_pool_ * 1.5), max_sets_limit);
    new_pool = createPool(ratios_);
  }
  return new_pool;
}

VkDescriptorPool
DescriptorAllocator::createPool(std::span<PoolSizeRatio> pool_ratios)
{
  std::vector<VkDescriptorPoolSize> pool_sizes;
  for (PoolSizeRatio ratio : pool_ratios) {
    pool_sizes.push_back({ .type            = ratio.type,
                           .descriptorCount = static_cast<uint32_t>(
                             ratio.ratio * sets_per_pool_) });
  }

  const VkDescriptorPoolCreateInfo pool_ci{
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = {},
    .flags         = 0,
    .maxSets       = sets_per_pool_,
    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
    .pPoolSizes    = pool_sizes.data(),
  };

  VkDescriptorPool new_pool;
  if (const VkResult result =
        vkCreateDescriptorPool(device_.logical(), &pool_ci, nullptr, &new_pool);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateDescriptorPool(): ");
  return new_pool;
}

void DescriptorAllocator::resetPools()
{
  for (auto p : ready_pools_)
    vkResetDescriptorPool(device_.logical(), p, 0);
  for (auto p : full_pools_) {
    vkResetDescriptorPool(device_.logical(), p, 0);
    ready_pools_.push_back(p);
  }
  full_pools_.clear();
}

void DescriptorAllocator::destroyPools()
{
  for (auto p : ready_pools_)
    vkDestroyDescriptorPool(device_.logical(), p, nullptr);
  ready_pools_.clear();
  for (auto p : full_pools_)
    vkDestroyDescriptorPool(device_.logical(), p, nullptr);
  full_pools_.clear();
}

VkDescriptorSet DescriptorAllocator::allocate(VkDescriptorSetLayout layout,
                                              void*                 pNext)
{
  /**
   * From Vulkan tutorial:
   *
   * Inadequate descriptor pools are a good example of a problem that the
   * validation layers will not catch: As of Vulkan 1.1,
   * vkAllocateDescriptorSets may fail with the error code
   * VK_ERROR_POOL_OUT_OF_MEMORY if the pool is not sufficiently large, but the
   * driver may also try to solve the problem internally. This means that
   * sometimes (depending on hardware, pool size and allocation size) the driver
   * will let us get away with an allocation that exceeds the limits of our
   * descriptor pool. Other times, vkAllocateDescriptorSets will fail and return
   * VK_ERROR_POOL_OUT_OF_MEMORY. This can be particularly frustrating if the
   * allocation succeeds on some machines, but fails on others.
   */

  // get or create a pool to allocate from
  VkDescriptorPool pool_to_use{ getPool() };

  VkDescriptorSetAllocateInfo alloc_info{
    .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext              = pNext,
    .descriptorPool     = pool_to_use,
    .descriptorSetCount = 1,
    .pSetLayouts        = &layout,
  };

  VkDescriptorSet ds;
  VkResult result{ vkAllocateDescriptorSets(device_.logical(), &alloc_info,
                                            &ds) };
  // allocation failed. Try again
  if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
      result == VK_ERROR_FRAGMENTED_POOL) {
    full_pools_.push_back(pool_to_use);
    pool_to_use               = getPool();
    alloc_info.descriptorPool = pool_to_use;
    if (result = vkAllocateDescriptorSets(device_.logical(), &alloc_info, &ds);
        result != VK_SUCCESS)
      ThrowVk(result, "vkAllocateDescriptorSets(): ");
  }
  else if (result != VK_SUCCESS) {
    ThrowVk(result, "vkAllocateDescriptorSets(): ");
  }

  ready_pools_.push_back(pool_to_use);
  return ds;
}

} // namespace eldr::vk::wr
