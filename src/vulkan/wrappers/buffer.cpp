#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

GpuBuffer::GpuBuffer(const Device& device, VkDeviceSize buffer_size,
                     VkBufferUsageFlags buffer_usage,
                     VmaMemoryUsage memory_usage, const std::string& name)
  : GpuResource(device, name), size_(buffer_size)
{
  const VkBufferCreateInfo buffer_ci{
    .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext                 = {},
    .flags                 = {},
    .size                  = size_,
    .usage                 = buffer_usage,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = {},
  };

  const VmaAllocationCreateInfo alloc_ci{
    .flags          = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    .usage          = memory_usage,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };

  if (const VkResult result =
        vmaCreateBuffer(device_.allocator(), &buffer_ci, &alloc_ci, &buffer_,
                        &allocation_, &alloc_info_);
      result != VK_SUCCESS)
    ThrowVk(result, "vmaCreateBuffer(): ");

  vmaSetAllocationName(device_.allocator(), allocation_, name.c_str());
}

GpuBuffer::GpuBuffer(const Device& device, const void* data,
                     VkDeviceSize data_size, VkBufferUsageFlags buffer_usage,
                     VmaMemoryUsage memory_usage, const std::string& name)
  : GpuBuffer(device, data_size, buffer_usage, memory_usage, name)
{
  assert(data_size > 0);
  assert(data);
  uploadData(data, data_size);
}

GpuBuffer::GpuBuffer(GpuBuffer&& other) noexcept
  : GpuResource(other.device_, other.name_)
{
  allocation_ = std::exchange(other.allocation_, {});
  alloc_info_ = std::exchange(other.alloc_info_, {});
  buffer_     = std::exchange(other.buffer_, VK_NULL_HANDLE);
  size_       = std::exchange(other.size_, 0);
}

GpuBuffer::~GpuBuffer()
{
  if (buffer_ != VK_NULL_HANDLE)
    vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
}

void GpuBuffer::uploadData(const void* data, size_t data_size)
{
  assert(static_cast<VkDeviceSize>(data_size) <= size_);
  assert(alloc_info_.pMappedData != nullptr);
  std::memcpy(alloc_info_.pMappedData, data, data_size);
}

} // namespace eldr::vk::wr
