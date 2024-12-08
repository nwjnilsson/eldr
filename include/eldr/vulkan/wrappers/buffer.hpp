#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/vktypes.hpp>

namespace eldr::vk::wr {
class Buffer {
public:
  Buffer() = default;
  Buffer(
    const Device& device, std::string_view name, VkDeviceSize buffer_size,
    VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage,
    VmaAllocationCreateFlags alloc_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT);

  Buffer(
    const Device&, std::string_view name, const void* data,
    VkDeviceSize data_size, VkBufferUsageFlags buffer_usage,
    VmaMemoryUsage           memory_usage,
    VmaAllocationCreateFlags alloc_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT);

  // VkDeviceSize    size() const { return size_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkBuffer           get() const;
  /// @brief Returns the exact size requested upon buffer creation. The real
  /// size of the VMA allocation may be greater. See VMA allocation creation
  /// docs for more info.
  /// @return VkDeviceSize The size requested upon buffer creation, in bytes.
  [[nodiscard]] VkDeviceSize size() const { return size_; }

  /// @brief Returns whether the buffer is empty or not.
  /// @return bool `true` if size_ == 0.
  [[nodiscard]] bool empty() const { return size_ == 0; }

  /// @brief Copies the data pointed to by `data` to the mapped GPU memory.
  /// @param data Pointer to the data to copy to the GPU buffer.
  /// @param data_size The size, in bytes, to copy.
  void uploadData(const void* data, size_t data_size) const;
  // void copyFromBuffer(const GpuBuffer&, const CommandPool&);

private:
  std::string name_;
  class BufferImpl;
  std::shared_ptr<BufferImpl> buffer_data_;
  VkDeviceSize                size_{ 0 };
};
} // namespace eldr::vk::wr
