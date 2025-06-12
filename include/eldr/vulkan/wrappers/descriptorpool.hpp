#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)
class DescriptorPool {
public:
  DescriptorPool();
  DescriptorPool(const Device&                         device,
                 uint32_t                              max_sets,
                 std::span<const VkDescriptorPoolSize> pool_sizes,
                 VkDescriptorPoolCreateFlags           flags = 0);
  DescriptorPool(DescriptorPool&&) noexcept;
  ~DescriptorPool();

  DescriptorPool& operator=(DescriptorPool&&);

  [[nodiscard]] VkDescriptorPool vk() const;
  void                           reset(VkDescriptorPoolResetFlags flags = 0);

private:
  class DescriptorPoolImpl;
  std::unique_ptr<DescriptorPoolImpl> d_;
};
NAMESPACE_END(eldr::vk::wr)
