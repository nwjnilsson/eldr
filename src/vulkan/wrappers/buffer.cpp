#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// BufferImpl
//------------------------------------------------------------------------------
class Buffer::BufferImpl : public GpuResourceAllocation {
public:
  BufferImpl(const Device& device, const VkBufferCreateInfo& buffer_ci,
             const VmaAllocationCreateInfo& alloc_ci);
  ~BufferImpl();
  VkBuffer buffer_{ VK_NULL_HANDLE };
};

Buffer::BufferImpl::BufferImpl(const Device&                  device,
                               const VkBufferCreateInfo&      buffer_ci,
                               const VmaAllocationCreateInfo& alloc_ci)
  : GpuResourceAllocation(device)
{
  if (const VkResult result =
        vmaCreateBuffer(device_.allocator(), &buffer_ci, &alloc_ci, &buffer_,
                        &allocation_, &alloc_info_);
      result != VK_SUCCESS)
    ThrowVk(result, "vmaCreateBuffer(): ");
}

Buffer::BufferImpl::~BufferImpl()
{
  vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
}

//------------------------------------------------------------------------------
// Buffer
//------------------------------------------------------------------------------
Buffer::Buffer(const Device& device, std::string_view name,
               VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage,
               VmaMemoryUsage           memory_usage,
               VmaAllocationCreateFlags alloc_flags)
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
    .flags          = alloc_flags,
    .usage          = memory_usage,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };
  buffer_data_ =
    std::make_shared<BufferImpl>(device, buffer_ci, alloc_ci, name);
  vmaSetAllocationName(device.allocator(), buffer_data_->allocation_,
                       fmt::format("{} allocation", name).c_str());
}

Buffer::Buffer(const Device& device, std::string_view name, const void* data,
               VkDeviceSize data_size, VkBufferUsageFlags buffer_usage,
               VmaMemoryUsage           memory_usage,
               VmaAllocationCreateFlags alloc_flags)
  : Buffer(device, name, data_size, buffer_usage, memory_usage, alloc_flags)
{
  assert(data_size > 0);
  assert(data);
  uploadData(data, data_size);
}

void Buffer::uploadData(const void* data, size_t data_size)
{
  assert(data_size > 0);
  assert(static_cast<VkDeviceSize>(data_size) <= size_);
  assert(buffer_data_->alloc_info_.pMappedData != nullptr);
  std::memcpy(buffer_data_->alloc_info_.pMappedData, data, data_size);
}

VkBuffer Buffer::get() const { return buffer_data_->buffer_; }
} // namespace eldr::vk::wr
