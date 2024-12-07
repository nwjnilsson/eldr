#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/wrappers/descriptorpool.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk {

DescriptorAllocator::DescriptorAllocator(const wr::Device&        device,
                                         uint32_t                 max_sets,
                                         std::span<PoolSizeRatio> ratios)
  : device_(device), ratios_(ratios.begin(), ratios.end()),
    sets_per_pool_(std::min(max_sets, max_sets_limit))
{
  ready_pools_.push_back(createPool());
}

DescriptorAllocator::~DescriptorAllocator() { destroyPools(); }

wr::DescriptorPool DescriptorAllocator::getPool()
{
  if (!ready_pools_.empty()) {
    wr::DescriptorPool new_pool{ std::move(ready_pools_.back()) };
    ready_pools_.pop_back();
    return new_pool;
  }
  else {
    // need to create a new pool
    return createPool();
  }
}

wr::DescriptorPool DescriptorAllocator::createPool()
{
  // Update max sets per pool for the creation of this pool
  sets_per_pool_ =
    std::min(static_cast<uint32_t>(sets_per_pool_ * 1.5), max_sets_limit);

  std::vector<VkDescriptorPoolSize> pool_sizes;
  for (PoolSizeRatio ratio : ratios_) {
    pool_sizes.push_back({ .type            = ratio.type,
                           .descriptorCount = static_cast<uint32_t>(
                             ratio.ratio * sets_per_pool_) });
  }

  return wr::DescriptorPool{ device_, sets_per_pool_, pool_sizes };
}

void DescriptorAllocator::resetPools()
{
  for (auto& p : ready_pools_)
    p.reset();
  for (auto& p : full_pools_) {
    p.reset();
    ready_pools_.push_back(std::move(p));
  }
  full_pools_.clear();
}

void DescriptorAllocator::destroyPools()
{
  ready_pools_.clear();
  full_pools_.clear();
}

VkDescriptorSet
DescriptorAllocator::allocate(const wr::DescriptorSetLayout& layout,
                              void*                          pNext)
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
  wr::DescriptorPool pool_to_use{ getPool() };

  VkDescriptorSetLayout       layouts[]{ layout.get() };
  VkDescriptorSetAllocateInfo alloc_info{
    .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext              = pNext,
    .descriptorPool     = pool_to_use.get(),
    .descriptorSetCount = 1,
    .pSetLayouts        = layouts,
  };

  VkDescriptorSet ds;
  VkResult result{ vkAllocateDescriptorSets(device_.logical(), &alloc_info,
                                            &ds) };
  // allocation failed. Try again
  if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
      result == VK_ERROR_FRAGMENTED_POOL) {
    full_pools_.push_back(std::move(pool_to_use));
    pool_to_use               = getPool();
    alloc_info.descriptorPool = pool_to_use.get();
    if (result = vkAllocateDescriptorSets(device_.logical(), &alloc_info, &ds);
        result != VK_SUCCESS)
      ThrowVk(result, "vkAllocateDescriptorSets(): ");
  }
  else if (result != VK_SUCCESS) {
    ThrowVk(result, "vkAllocateDescriptorSets(): ");
  }

  ready_pools_.push_back(std::move(pool_to_use));
  return ds;
}

} // namespace eldr::vk
