#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/wrappers/descriptorpool.hpp>

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
  VkDescriptorSet allocate(const Device&              device,
                           const DescriptorSetLayout& layout,
                           void*                          pNext = nullptr);

private:
  DescriptorPool getPool(const Device& device);
  DescriptorPool createPool(const Device& device);

private:
  static constexpr uint32_t max_sets_limit{ 4092 };

  std::vector<PoolSizeRatio>      ratios_;
  std::vector<DescriptorPool> full_pools_;
  std::vector<DescriptorPool> ready_pools_;
  uint32_t                        sets_per_pool_;
};
NAMESPACE_END(eldr::vk)
