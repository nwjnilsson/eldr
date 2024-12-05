#pragma once

#include <eldr/vulkan/common.hpp>

#include <span>

namespace eldr::vk::wr {
class DescriptorAllocator {
public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    float            ratio;
  };

  DescriptorAllocator()                           = delete;
  DescriptorAllocator(const DescriptorAllocator&) = delete;
  DescriptorAllocator(DescriptorAllocator&&)      = delete;
  DescriptorAllocator(const Device& device, uint32_t max_sets,
                      std::span<PoolSizeRatio> ratios);
  ~DescriptorAllocator();

  void            resetPools();
  void            destroyPools();
  VkDescriptorSet allocate(VkDescriptorSetLayout layout, void* pNext = nullptr);

private:
  VkDescriptorPool getPool();
  VkDescriptorPool createPool(std::span<PoolSizeRatio> pool_ratios);

private:
  static constexpr uint32_t max_sets_limit{ 4092 };

  const Device& device_;

  std::vector<PoolSizeRatio>    ratios_;
  std::vector<VkDescriptorPool> full_pools_;
  std::vector<VkDescriptorPool> ready_pools_;
  uint32_t                      sets_per_pool_;
};
} // namespace eldr::vk::wr
