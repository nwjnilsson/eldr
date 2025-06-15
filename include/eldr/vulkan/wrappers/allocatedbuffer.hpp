#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)
class AllocatedBuffer : public VkAllocatedObject<VkBuffer> {
  using Base = VkAllocatedObject<VkBuffer>;

public:
  AllocatedBuffer();
  AllocatedBuffer(AllocatedBuffer&&) noexcept;
  virtual ~AllocatedBuffer();

  AllocatedBuffer& operator=(AllocatedBuffer&&);

  /// @brief Returns the size (capacity), in bytes, of the memory allocation.
  [[nodiscard]] size_t allocSize() const { return alloc_info_.size; }

  /// @brief Returns a VkDeviceAddress for this buffer
  [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

protected:
  AllocatedBuffer(std::string_view         name,
                  const Device&            device,
                  size_t                   size_bytes,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags host_access,
                  VmaMemoryUsage           mem_usage);

  void uploadData(std::span<const byte_t> src, size_t offset = 0);

protected:
  VmaAllocation         allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo     alloc_info_;
  VkMemoryPropertyFlags mem_flags_;
};

NAMESPACE_END(eldr::vk::wr)
