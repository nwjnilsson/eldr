#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)
class AllocatedBuffer : VkAllocatedObject<VkBuffer> {
  using Base = VkAllocatedObject<VkBuffer>;

public:
  AllocatedBuffer();
  AllocatedBuffer(AllocatedBuffer&&) noexcept;
  virtual ~AllocatedBuffer();

  AllocatedBuffer& operator=(AllocatedBuffer&&) noexcept;

  /// @brief Returns the minimum capacity, in bytes, of the buffer.
  [[nodiscard]] size_t sizeBytes() const { return size_bytes_; }
  /// @brief Returns the size (capacity), in bytes, of the memory allocation.
  [[nodiscard]] size_t sizeAlloc() const;
  /// @brief Returns a VkDeviceAddress for this buffer
  [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

protected:
  AllocatedBuffer(const Device&            device,
                  std::string_view         name,
                  size_t                   size_bytes,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags host_access,
                  VmaMemoryUsage           mem_usage);

  void uploadData(std::span<const byte_t> src);

protected:
  size_t                size_bytes_;
  VmaAllocation         allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo     alloc_info_;
  VkMemoryPropertyFlags mem_flags_;
};

NAMESPACE_END(eldr::vk::wr)
