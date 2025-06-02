#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
class AllocatedBuffer {
public:
  AllocatedBuffer() = default;

  [[nodiscard]] const char* name() const;
  [[nodiscard]] VkBuffer    vk() const;
  /// @brief Returns the capacity, in number of elements, of the buffer.
  [[nodiscard]] size_t size() const { return size_; }
  /// @brief Returns the size of individual elements in the buffer.
  [[nodiscard]] size_t sizeElem() const { return elem_size_; }
  /// @brief Returns the minimum capacity, in bytes, of the buffer.
  [[nodiscard]] size_t sizeBytes() const { return size() * sizeElem(); }
  /// @brief Returns whether the buffer is empty or not.
  [[nodiscard]] bool empty() const { return size_ == 0; }
  /// @brief Returns the size (capacity), in bytes, of the memory allocation.
  [[nodiscard]] size_t sizeAlloc() const;
  /// @brief Returns a VkDeviceAddress for this buffer
  [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

protected:
  AllocatedBuffer(const Device&      device,
                  const std::string& name,
                  size_t             elem_size,
                  size_t             elem_count,
                  BufferUsageFlags   buffer_usage,
                  HostAccessFlags    host_access,
                  MemoryUsage        mem_usage);

protected:
  std::string name_;
  size_t      size_{ 0 };
  size_t      elem_size_{ 0 };
  class BufferImpl;
  std::shared_ptr<BufferImpl> d_;
};

class AllocatedBuffer::BufferImpl : public GpuResourceAllocation {
  friend AllocatedBuffer;

public:
  BufferImpl(const Device&                  device,
             const VkBufferCreateInfo&      buffer_ci,
             const VmaAllocationCreateInfo& alloc_ci)
    : GpuResourceAllocation(device, {}, {}, MemoryPropertyFlags{})
  {
    const VkResult result{ vmaCreateBuffer(device_.allocator(),
                                           &buffer_ci,
                                           &alloc_ci,
                                           &buffer_,
                                           &allocation_,
                                           &alloc_info_) };
    if (result != VK_SUCCESS) {
      Throw("Failed to create buffer ({})", result);
    }
    VkMemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(device.allocator(), allocation_, &flags);
    mem_flags_ = static_cast<MemoryPropertyFlags>(flags);
  }

  ~BufferImpl() { vmaDestroyBuffer(device_.allocator(), buffer_, allocation_); }

private:
  VkBuffer buffer_{ VK_NULL_HANDLE };
};

} // namespace eldr::vk::wr
