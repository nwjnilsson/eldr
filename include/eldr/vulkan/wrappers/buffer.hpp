#pragma once
#include <eldr/vulkan/wrappers/allocatedbuffer.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)

// Should probably support uploading to a specific section of the buffer, and
// not just overwriting from the beginning of the mapped data
template <typename T> class Buffer final : public AllocatedBuffer {
public:
  Buffer() = default;
  Buffer(const Device&            device,
         const std::string&       name,
         size_t                   elem_count,
         VkBufferUsageFlags       buffer_usage,
         VmaAllocationCreateFlags allocation_flags = 0,
         VmaMemoryUsage                            = VMA_MEMORY_USAGE_AUTO);
  Buffer(const Device&            device,
         const std::string&       name,
         std::span<const T>       data,
         VkBufferUsageFlags       buffer_usage,
         VmaAllocationCreateFlags allocation_flags = 0,
         VmaMemoryUsage           mem_usage        = VMA_MEMORY_USAGE_AUTO);
  Buffer(Buffer&&) noexcept = default;
  ~Buffer()                 = default;

  Buffer& operator=(Buffer&&) = default;

  /// @brief Returns the capacity, in number of elements, of the buffer.
  [[nodiscard]] size_t size() const { return size_; }
  /// @brief Returns whether the buffer is empty or not.
  [[nodiscard]] bool empty() const { return size_ == 0; }

  /// @brief Copies all contents of `data` into this buffer.
  /// @param data The span of data to upload to the GPU buffer.
  void uploadData(std::span<const T> src);

private:
  size_t size_{ 0 };
};

template <typename T>
Buffer<T>::Buffer(const Device&            device,
                  const std::string&       name,
                  size_t                   elem_count,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags allocation_flags,
                  VmaMemoryUsage           mem_usage)
  : AllocatedBuffer(device,
                    name,
                    sizeof(T) * elem_count,
                    buffer_usage,
                    allocation_flags,
                    mem_usage),
    size_(elem_count)
{
}

template <typename T>
Buffer<T>::Buffer(const Device&            device,
                  const std::string&       name,
                  std::span<const T>       data,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags allocation_flags,
                  VmaMemoryUsage           mem_usage)
  : Buffer(device, name, data.size(), buffer_usage, allocation_flags, mem_usage)
{
  uploadData(data);
}

template <typename T> void Buffer<T>::uploadData(std::span<const T> src)
{
  if (src.empty()) {
    return;
  }
  AllocatedBuffer::uploadData(std::as_bytes(src));
  size_ = src.size();
}

NAMESPACE_END(eldr::vk::wr)
