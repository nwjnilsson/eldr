#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

namespace eldr::vk::wr {
class AllocatedBuffer::BufferImpl : public GpuResourceAllocation {
public:
  BufferImpl(const Device&                  device,
             const VkBufferCreateInfo&      buffer_ci,
             const VmaAllocationCreateInfo& alloc_ci)
    : GpuResourceAllocation(device, {}, {}, MemoryPropertyFlags{})
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
    VkMemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(device.allocator(), allocation_, &flags);
    mem_flags_ = static_cast<MemoryPropertyFlags>(flags);
  }

  ~BufferImpl() { vmaDestroyBuffer(device_.allocator(), buffer_, allocation_); }

  VkBuffer buffer_{ VK_NULL_HANDLE };
};

AllocatedBuffer::AllocatedBuffer(const Device&      device,
                                 const std::string& name,
                                 size_t             size,
                                 BufferUsageFlags   buffer_usage,
                                 HostAccessFlags    host_access,
                                 MemoryUsage        mem_usage)
  : size_(size)
{
  Assert(size_ > 0);
  const VkBufferCreateInfo buffer_ci{
    .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext                 = {},
    .flags                 = {},
    .size                  = size,
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

void AllocatedBuffer::uploadData(std::span<const byte_t> src)
{
  if (src.empty()) {
    return;
  }
  if (src.size_bytes() > d_->alloc_info_.size) {
    Throw("Buffer size is too small.");
  }

  if (d_->mem_flags_.hasFlag(MemoryProperty::HostVisible)) {
    // Host visible buffer, map memory and memcpy
    Assert(d_->mem_flags_.hasFlag(MemoryProperty::HostCoherent)); // fow now
    // if not HOST_COHERENT, a flush is needed using vmaInvalidateAllocation()
    // / vmaFlushAllocation()
    void* dst{ nullptr };
    vmaMapMemory(d_->device_.allocator(), d_->allocation_, &dst);
    Assert(dst);
    std::memcpy(dst, src.data(), src.size_bytes());
    vmaUnmapMemory(d_->device_.allocator(), d_->allocation_);
  }
  else {
    d_->device_.execute([&](const CommandBuffer& cb) {
      const VkBufferCopy2 copy_regions[]{ {
        .sType     = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
        .pNext     = {},
        .srcOffset = 0,
        .dstOffset = 0,
        .size      = src.size_bytes(),
      } };
      cb.copyDataToBuffer(*this, src, copy_regions);
    });
  }
  size_ = src.size();
}
} // namespace eldr::vk::wr
