#pragma once
#include <eldr/vulkan/wrappers/allocatedbuffer.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)

// Should probably support uploading to a specific section of the buffer, and
// not just overwriting from the beginning of the mapped data
template <typename T> class Buffer final : public AllocatedBuffer {
public:
  Buffer() = default;
  Buffer(std::string_view         name,
         const Device&            device,
         size_t                   elem_count,
         VkBufferUsageFlags       buffer_usage,
         VmaAllocationCreateFlags allocation_flags = 0,
         VmaMemoryUsage                            = VMA_MEMORY_USAGE_AUTO);
  Buffer(std::string_view         name,
         const Device&            device,
         std::span<const T>       data,
         VkBufferUsageFlags       buffer_usage,
         VmaAllocationCreateFlags allocation_flags = 0,
         VmaMemoryUsage           mem_usage        = VMA_MEMORY_USAGE_AUTO);
  Buffer(Buffer&&) noexcept = default;
  ~Buffer()                 = default;

  Buffer& operator=(Buffer&&) = default;

  /// Returns the capacity requested upon buffer creation, in number of
  /// elements.
  [[nodiscard]] size_t size() const { return size_; }
  /// Returns the capacity requested upon buffer creation, in bytes.
  [[nodiscard]] size_t sizeBytes() const { return size_ * sizeof(T); }
  /// @brief Returns whether the buffer is empty or not.
  [[nodiscard]] bool empty() const { return size_ == 0; }

  /// @brief Copies all contents of `data` into this buffer.
  /// @param data The span of data to upload to the GPU buffer.
  /// @param offset Data index offset
  void uploadData(std::span<const T> src, size_t offset = 0);

private:
  size_t size_{ 0 };
};

template <typename T>
Buffer<T>::Buffer(std::string_view         name,
                  const Device&            device,
                  size_t                   elem_count,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags allocation_flags,
                  VmaMemoryUsage           mem_usage)
  : AllocatedBuffer(name,
                    device,
                    sizeof(T) * elem_count,
                    buffer_usage,
                    allocation_flags,
                    mem_usage),
    size_(elem_count)
{
}

template <typename T>
Buffer<T>::Buffer(std::string_view         name,
                  const Device&            device,
                  std::span<const T>       data,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags allocation_flags,
                  VmaMemoryUsage           mem_usage)
  : Buffer(name, device, data.size(), buffer_usage, allocation_flags, mem_usage)
{
  uploadData(data);
}

template <typename T>
void Buffer<T>::uploadData(std::span<const T> src, size_t offset)
{
  if (src.empty()) {
    return;
  }
  AllocatedBuffer::uploadData(std::as_bytes(src), offset * sizeof(T));
}

NAMESPACE_END(eldr::vk::wr)
