#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/vertex.hpp>
#include <eldr/vulkan/wrappers/gpuresource.hpp>

namespace eldr::vk::wr {

struct BufferCreateInfo {
  VkDeviceSize       buffer_size;
  VkBufferUsageFlags usage;
};

class GpuBuffer final : public GpuResource {
public:
  // GpuBuffer(const Device&, const BufferResource&,
  //           const VmaAllocationCreateInfo&, const std::string& name);
  GpuBuffer(const Device&, VkDeviceSize buffer_size,
            VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage,
            const std::string& name);
  GpuBuffer(const Device&, VkDeviceSize buffer_size, const void* data,
            size_t data_size, VkBufferUsageFlags buffer_usage,
            VmaMemoryUsage memory_usage, const std::string& name);
  // GpuBuffer(const Device&, const std::vector<Vertex>&,
  //        const CommandPool&); // Vertex buffer
  // GpuBuffer(const Device&, const std::vector<uint32_t>&,
  //        const CommandPool&); // Index buffer
  GpuBuffer(GpuBuffer&&) noexcept;
  ~GpuBuffer();

  GpuBuffer& operator=(const GpuBuffer&) = delete;
  GpuBuffer& operator=(GpuBuffer&&)      = delete;

  // VkDeviceSize    size() const { return size_; }
  const VkBuffer& get() const { return buffer_; }
  // const VkDeviceMemory& memory() const { return buffer_memory_; }
  void uploadData(const void* data, size_t size);
  void copyFromBuffer(const GpuBuffer&, const CommandPool&);

private:
  VkBuffer     buffer_{ VK_NULL_HANDLE };
  VkDeviceSize size_{ 0 };
};
} // namespace eldr::vk::wr
