#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// BufferImpl
//------------------------------------------------------------------------------
class AllocatedBuffer::BufferImpl : public GpuResourceAllocation {
  friend AllocatedBuffer;

public:
  BufferImpl(const Device&                  device,
             const VkBufferCreateInfo&      buffer_ci,
             const VmaAllocationCreateInfo& alloc_ci)
    : GpuResourceAllocation(device, {}, {}, {})
  {
    const VkResult result{ vmaCreateBuffer(device_.allocator(),
                                           &buffer_ci,
                                           &alloc_ci,
                                           &buffer_,
                                           &allocation_,
                                           &alloc_info_) };
    if (result != VK_SUCCESS) {
      Throw("Failed to create buffer ({})", result);
    }
    vmaGetAllocationMemoryProperties(
      device.allocator(), allocation_, &mem_flags_);
  }

  ~BufferImpl() { vmaDestroyBuffer(device_.allocator(), buffer_, allocation_); }

private:
  VkBuffer buffer_{ VK_NULL_HANDLE };
};

//------------------------------------------------------------------------------
// AllocatedBuffer
//------------------------------------------------------------------------------
AllocatedBuffer::AllocatedBuffer()                             = default;
AllocatedBuffer::~AllocatedBuffer()                            = default;
AllocatedBuffer::AllocatedBuffer(AllocatedBuffer&&) noexcept   = default;
AllocatedBuffer& AllocatedBuffer::operator=(AllocatedBuffer&&) = default;

AllocatedBuffer::AllocatedBuffer(const Device&            device,
                                 const std::string&       name,
                                 size_t                   size_bytes,
                                 VkBufferUsageFlags       buffer_usage,
                                 VmaAllocationCreateFlags allocation_flags,
                                 VmaMemoryUsage           mem_usage)
  : name_(name), size_bytes_(size_bytes)
{
  Assert(size_bytes > 0);
  const VkBufferCreateInfo buffer_ci{
    .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext                 = {},
    .flags                 = {},
    .size                  = size_bytes_,
    .usage                 = buffer_usage,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = {},
  };

  const VmaAllocationCreateInfo alloc_ci{
    .flags          = allocation_flags,
    .usage          = static_cast<VmaMemoryUsage>(mem_usage),
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };
  d_ = std::make_unique<BufferImpl>(device, buffer_ci, alloc_ci);
  vmaSetAllocationName(device.allocator(), d_->allocation_, name.c_str());

#ifdef DEBUG
  if ((allocation_flags &
       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) or
      (allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) or
      (allocation_flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)) {
    Assert(d_->mem_flags_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  }
#endif
}

VkBuffer AllocatedBuffer::vk() const { return d_->buffer_; }
size_t   AllocatedBuffer::sizeAlloc() const { return d_->alloc_info_.size; }

VkDeviceAddress AllocatedBuffer::getDeviceAddress() const
{
  VkBufferDeviceAddressInfo address_info{
    .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    .pNext  = {},
    .buffer = d_->buffer_
  };
  return vkGetBufferDeviceAddress(d_->device_.logical(), &address_info);
}

void AllocatedBuffer::uploadData(std::span<const byte_t> src)
{
  if (d_->mem_flags_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    if (src.size_bytes() > sizeAlloc()) {
      Throw("Buffer size is too small.");
    }
    // Host visible buffer, map memory and memcpy
    Assert(d_->mem_flags_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // fow now
    // TODO: if not HOST_COHERENT, a flush is needed using
    // vmaInvalidateAllocation() / vmaFlushAllocation()
    void* dst{ nullptr };
    vmaMapMemory(d_->device_.allocator(), d_->allocation_, &dst);
    Assert(dst);
    std::memcpy(dst, src.data(), src.size_bytes());
    vmaUnmapMemory(d_->device_.allocator(), d_->allocation_);
  }
  else {
    d_->device_.execute(
      [&](const CommandBuffer& cb) { cb.copyDataToBuffer(*this, src); });
  }
  size_bytes_ = src.size_bytes();
}
} // namespace eldr::vk::wr
