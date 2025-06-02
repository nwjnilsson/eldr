#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

namespace eldr::vk::wr {
AllocatedBuffer::AllocatedBuffer(const Device&      device,
                                 const std::string& name,
                                 size_t             elem_size,
                                 size_t             elem_count,
                                 BufferUsageFlags   buffer_usage,
                                 HostAccessFlags    host_access,
                                 MemoryUsage        mem_usage)
  : size_(elem_count), elem_size_(elem_size)
{
  const VkBufferCreateInfo buffer_ci{
    .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext                 = {},
    .flags                 = {},
    .size                  = size_ * elem_size_,
    .usage                 = buffer_usage.flags,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = {},
  };

  const VmaAllocationCreateInfo alloc_ci{
    .flags          = host_access.flags,
    .usage          = static_cast<VmaMemoryUsage>(mem_usage),
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };
  d_ = std::make_shared<BufferImpl>(device, buffer_ci, alloc_ci);
  vmaSetAllocationName(device.allocator(), d_->allocation_, name.c_str());

#ifdef DEBUG
  if (host_access.hasFlag(HostAccess::Sequential) or
      host_access.hasFlag(HostAccess::Random)) {
    Assert(d_->mem_flags_.hasFlag(MemoryProperty::HostVisible));
  }
#endif
}

const char* AllocatedBuffer::name() const { return d_->alloc_info_.pName; }
VkBuffer    AllocatedBuffer::vk() const { return d_->buffer_; }
size_t      AllocatedBuffer::sizeAlloc() const { return d_->alloc_info_.size; }

VkDeviceAddress AllocatedBuffer::getDeviceAddress() const
{
  VkBufferDeviceAddressInfo address_info{
    .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    .pNext  = {},
    .buffer = d_->buffer_
  };
  return vkGetBufferDeviceAddress(d_->device_.logical(), &address_info);
}
} // namespace eldr::vk::wr
