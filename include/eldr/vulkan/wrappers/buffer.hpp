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
  GpuBuffer(const Device&, VkDeviceSize buffer_size,
            VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage,
            const std::string& name);
  GpuBuffer(const Device&, const void* data, VkDeviceSize data_size,
            VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage,
            const std::string& name);
  GpuBuffer(GpuBuffer&&) noexcept;
  ~GpuBuffer();

  GpuBuffer& operator=(const GpuBuffer&) = delete;
  GpuBuffer& operator=(GpuBuffer&&)      = delete;

  // VkDeviceSize    size() const { return size_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkBuffer           get() const { return buffer_; }

  void uploadData(const void* data, size_t data_size);
  void copyFromBuffer(const GpuBuffer&, const CommandPool&);

private:
  VkBuffer     buffer_{ VK_NULL_HANDLE };
  VkDeviceSize size_{ 0 };
};
} // namespace eldr::vk::wr
