#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/vertex.hpp>

namespace eldr::vk::wr {

struct BufferInfo {
  VkDeviceSize          size;
  VkBufferUsageFlags    usage;
  VkMemoryPropertyFlags properties;
};

class Buffer {
public:
  Buffer(Device&, const BufferInfo&);
  Buffer(Device&, const std::vector<Vertex>&,
         CommandPool&); // Vertex buffer
  Buffer(Device&, const std::vector<uint32_t>&,
         CommandPool&); // Index buffer
  Buffer(Buffer&&);
  ~Buffer();

  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&&)      = delete;

  VkDeviceSize    size() const { return size_; }
  const VkBuffer& get() const { return buffer_; }
  // const VkDeviceMemory& memory() const { return buffer_memory_; }
  void copyFrom(Buffer&, CommandPool&);

protected:
  const Device& device_;

  VkBuffer buffer_{ VK_NULL_HANDLE };
  // VkDeviceMemory buffer_memory_{ VK_NULL_HANDLE };
  VkDeviceSize size_;
};
} // namespace eldr::vk::wr
