#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <span>

namespace eldr::vk::wr {

class AllocatedBuffer {
public:
  AllocatedBuffer() = default;

  [[nodiscard]] const char* name() const;
  [[nodiscard]] VkBuffer    vk() const;
  /// @brief size_t The number of elements in the buffer.
  [[nodiscard]] size_t size() const { return size_; }
  /// @brief Returns whether the buffer is empty or not.
  [[nodiscard]] bool empty() const { return size_ == 0; }
  /// @brief Returns the size (capacity), in bytes, of the memory allocation.
  [[nodiscard]] size_t sizeAlloc() const;
  /// @brief Returns a VkDeviceAddress for this buffer
  [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

protected:
  AllocatedBuffer(const Device&      device,
                  const std::string& name,
                  size_t             size,
                  BufferUsageFlags   buffer_usage,
                  HostAccessFlags    host_access,
                  MemoryUsage        mem_usage);
  void uploadData(std::span<const byte_t> data);

protected:
  std::string name_;
  size_t      size_{ 0 };
  class BufferImpl;
  std::shared_ptr<BufferImpl> d_;
};

// Should probably support uploading to a specific section of the buffer, and
// not just overwriting from the beginning of the mapped data
template <typename T> class Buffer : public AllocatedBuffer {
public:
  Buffer() = default;
  Buffer(const Device&      device,
         const std::string& name,
         size_t             elem_count,
         BufferUsageFlags   buffer_usage,
         HostAccessFlags    host_access,
         MemoryUsage        mem_usage = MemoryUsage::Auto)
    : AllocatedBuffer(device,
                      name,
                      sizeof(T) * elem_count,
                      buffer_usage,
                      host_access,
                      mem_usage)
  {
  }

  Buffer(const Device&      device,
         const std::string& name,
         std::span<const T> data,
         BufferUsageFlags   buffer_usage,
         HostAccessFlags    host_access,
         MemoryUsage        mem_usage = MemoryUsage::Auto)
    : Buffer(device, name, data.size(), buffer_usage, host_access, mem_usage)
  {
    uploadData(data);
  }

  /// @brief Returns the size, in bytes, of the elements in the buffer.
  [[nodiscard]] size_t sizeBytes() const { return size_ * sizeof(T); }

  /// @brief Copies all contents of `data` into this buffer.
  /// @param data The span of data to upload to the GPU buffer.
  void uploadData(std::span<const T> data)
  {
    AllocatedBuffer::uploadData(std::as_bytes(data));
  }
};
} // namespace eldr::vk::wr
