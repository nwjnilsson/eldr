#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/descriptorpool.hpp>

#include <span>

namespace eldr::vk {
class DescriptorAllocator {
public:
  struct PoolSizeRatio {
    VkDescriptorType type;
    float            ratio;
  };

  DescriptorAllocator() = default;
  DescriptorAllocator(uint32_t max_sets, std::span<PoolSizeRatio> ratios);

  void            resetPools();
  void            destroyPools();
  VkDescriptorSet allocate(const wr::Device&              device,
                           const wr::DescriptorSetLayout& layout,
                           void*                          pNext = nullptr);

private:
  wr::DescriptorPool getPool(const wr::Device& device);
  wr::DescriptorPool createPool(const wr::Device& device);

private:
  static constexpr uint32_t max_sets_limit{ 4092 };

  std::vector<PoolSizeRatio>      ratios_;
  std::vector<wr::DescriptorPool> full_pools_;
  std::vector<wr::DescriptorPool> ready_pools_;
  uint32_t                        sets_per_pool_;
};
} // namespace eldr::vk
