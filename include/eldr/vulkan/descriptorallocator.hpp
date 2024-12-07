#pragma once

#include <eldr/vulkan/common.hpp>

#include <span>

namespace eldr::vk {
class DescriptorAllocator {
public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    float            ratio;
  };

  DescriptorAllocator()                           = delete;
  DescriptorAllocator(const DescriptorAllocator&) = delete;
  DescriptorAllocator(DescriptorAllocator&&)      = delete;
  DescriptorAllocator(const wr::Device& device, uint32_t max_sets,
                      std::span<PoolSizeRatio> ratios);
  ~DescriptorAllocator();

  void            resetPools();
  void            destroyPools();
  VkDescriptorSet allocate(const wr::DescriptorSetLayout& layout,
                           void*                          pNext = nullptr);

private:
  wr::DescriptorPool getPool();
  wr::DescriptorPool createPool();

private:
  static constexpr uint32_t max_sets_limit{ 4092 };

  const wr::Device& device_;

  std::vector<PoolSizeRatio>      ratios_;
  std::vector<wr::DescriptorPool> full_pools_;
  std::vector<wr::DescriptorPool> ready_pools_;
  uint32_t                        sets_per_pool_;
};
} // namespace eldr::vk
