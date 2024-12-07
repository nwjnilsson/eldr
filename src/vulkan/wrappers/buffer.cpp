#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// GpuBufferImpl
//------------------------------------------------------------------------------
class GpuBuffer::GpuBufferImpl : public GpuResourceAllocation {
public:
  GpuBufferImpl(const Device& device, const VkBufferCreateInfo& buffer_ci,
                const VmaAllocationCreateInfo& alloc_ci);
  ~GpuBufferImpl();
  VkBuffer buffer_{ VK_NULL_HANDLE };
};

GpuBuffer::GpuBufferImpl::GpuBufferImpl(const Device&             device,
                                        const VkBufferCreateInfo& buffer_ci,
                                        const VmaAllocationCreateInfo& alloc_ci)
  : GpuResourceAllocation(device)
{
  if (const VkResult result =
        vmaCreateBuffer(device_.allocator(), &buffer_ci, &alloc_ci, &buffer_,
                        &allocation_, &alloc_info_);
      result != VK_SUCCESS)
    ThrowVk(result, "vmaCreateBuffer(): ");
}

GpuBuffer::GpuBufferImpl::~GpuBufferImpl()
{
  vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
}

//------------------------------------------------------------------------------
// GpuBuffer
//------------------------------------------------------------------------------
GpuBuffer::GpuBuffer(const Device& device, std::string_view name,
                     VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage,
                     VmaMemoryUsage           memory_usage,
                     VmaAllocationCreateFlags flags)
  : size_(buffer_size)
{
  assert(buffer_size > 0);
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
    .flags          = flags,
    .usage          = memory_usage,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };
  buffer_data_ =
    std::make_shared<GpuBufferImpl>(device, buffer_ci, alloc_ci, name);
  vmaSetAllocationName(device.allocator(), buffer_data_->allocation_,
                       fmt::format("{} allocation", name).c_str());
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

void GpuBuffer::uploadData(const void* data, size_t data_size)
{
  assert(data_size > 0);
  assert(static_cast<VkDeviceSize>(data_size) <= size_);
  assert(buffer_data_->alloc_info_.pMappedData != nullptr);
  std::memcpy(buffer_data_->alloc_info_.pMappedData, data, data_size);
}

VkBuffer GpuBuffer::get() const { return buffer_data_->buffer_; }
} // namespace eldr::vk::wr
