#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <span>

namespace eldr::vk::wr {
class AllocatedBuffer {
public:
  AllocatedBuffer();
  AllocatedBuffer(AllocatedBuffer&&) noexcept;
  virtual ~AllocatedBuffer();

  AllocatedBuffer& operator=(AllocatedBuffer&&);

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkBuffer           vk() const;
  /// @brief Returns the minimum capacity, in bytes, of the buffer.
  [[nodiscard]] size_t sizeBytes() const { return size_bytes_; }
  /// @brief Returns the size (capacity), in bytes, of the memory allocation.
  [[nodiscard]] size_t sizeAlloc() const;
  /// @brief Returns a VkDeviceAddress for this buffer
  [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

protected:
  AllocatedBuffer(const Device&            device,
                  const std::string&       name,
                  size_t                   size_bytes,
                  VkBufferUsageFlags       buffer_usage,
                  VmaAllocationCreateFlags host_access,
                  VmaMemoryUsage           mem_usage);

  void uploadData(std::span<const byte_t> src);

protected:
  std::string name_;
  size_t      size_bytes_;
  class BufferImpl;
  std::unique_ptr<BufferImpl> d_;
};

} // namespace eldr::vk::wr
