#pragma once
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/descriptorpool.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk)
struct PoolSizeRatio {
  VkDescriptorType type;
  float            ratio;
};

class DescriptorAllocator {
public:
  DescriptorAllocator() = default;
  DescriptorAllocator(uint32_t max_sets, std::span<const PoolSizeRatio> ratios);

  void resize(uint32_t max_sets);

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
NAMESPACE_END(eldr::vk)
