#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/vktypes.hpp>

namespace eldr::vk::wr {
class GpuBuffer {
public:
  GpuBuffer(const Device& device, std::string_view name,
            VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage,
            VmaMemoryUsage           memory_usage,
            VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT);
  GpuBuffer(const Device&, std::string_view name, const void* data,
            VkDeviceSize data_size, VkBufferUsageFlags buffer_usage,
            VmaMemoryUsage           memory_usage,
            VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT);

  // VkDeviceSize    size() const { return size_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkBuffer           get() const;
  /// @brief Returns the exact size requested upon buffer creation. The real
  /// size of the VMA allocation may be greater. See VMA allocation creation
  /// docs for more info.
  [[nodiscard]] VkDeviceSize size() const { return size_; }

  // More ways of uploading data? Templated maybe
  void uploadData(const void* data, size_t data_size);
  // void copyFromBuffer(const GpuBuffer&, const CommandPool&);

private:
  std::string name_{ "unknown" };
  class GpuBufferImpl;
  std::shared_ptr<GpuBufferImpl> buffer_data_;
  VkDeviceSize                   size_{ 0 };
};
} // namespace eldr::vk::wr
