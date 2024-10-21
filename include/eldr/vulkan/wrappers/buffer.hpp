#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vertex.hpp>

namespace eldr::vk::wr {

struct BufferInfo {
  VkDeviceSize          size;
  VkBufferUsageFlags    usage;
  VkMemoryPropertyFlags memory_flags;
};

class Buffer final : public vk::PhysicalResource {
public:
  Buffer(const Device&, const BufferResource&, const VmaAllocationCreateInfo&);
  Buffer(const Device&, const std::vector<Vertex>&,
         const CommandPool&); // Vertex buffer
  Buffer(const Device&, const std::vector<uint32_t>&,
         const CommandPool&); // Index buffer
  Buffer(Buffer&&);
  ~Buffer();

  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&&)      = delete;

  // VkDeviceSize    size() const { return size_; }
  const VkBuffer& get() const { return buffer_; }
  // const VkDeviceMemory& memory() const { return buffer_memory_; }
  void uploadData(const void* data, size_t size);
  void copyFromBuffer(const Buffer&, const CommandPool&);

protected:
  VkBuffer          buffer_{ VK_NULL_HANDLE };
  VmaAllocationInfo alloc_info_{};
};
} // namespace eldr::vk::wr
