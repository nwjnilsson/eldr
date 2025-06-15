#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(AllocatedBuffer)

AllocatedBuffer::AllocatedBuffer(std::string_view         name,
                                 const Device&            device,
                                 size_t                   size_bytes,
                                 VkBufferUsageFlags       buffer_usage,
                                 VmaAllocationCreateFlags allocation_flags,
                                 VmaMemoryUsage           mem_usage)
  : Base(name, device)
{
  Assert(size_bytes > 0);
  const VkBufferCreateInfo buffer_ci{
    .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext                 = {},
    .flags                 = {},
    .size                  = size_bytes,
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

  const VkResult result{ vmaCreateBuffer(device.allocator(),
                                         &buffer_ci,
                                         &alloc_ci,
                                         &object_,
                                         &allocation_,
                                         &alloc_info_) };
  if (result != VK_SUCCESS) {
    Throw("Failed to create buffer ({})", result);
  }
  vmaGetAllocationMemoryProperties(
    device.allocator(), allocation_, &mem_flags_);
  vmaSetAllocationName(device.allocator(), allocation_, this->name().c_str());
#ifdef DEBUG
  if ((allocation_flags &
       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) or
      (allocation_flags & VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT) or
      (allocation_flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)) {
    Assert(mem_flags_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  }
#endif
}

AllocatedBuffer::~AllocatedBuffer()
{
  if (vk()) {
    vmaDestroyBuffer(device().allocator(), vk(), allocation_);
  }
}

VkDeviceAddress AllocatedBuffer::getDeviceAddress() const
{
  VkBufferDeviceAddressInfo address_info{
    .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    .pNext  = {},
    .buffer = vk()
  };
  return vkGetBufferDeviceAddress(device().logical(), &address_info);
}

void AllocatedBuffer::uploadData(std::span<const byte_t> src, size_t offset)
{
  if (src.size() + offset > allocSize()) {
    Throw("Attempted to copy {} bytes (offset = {}) to buffer \"{}\" that has "
          "a total capacity of {} bytes",
          src.size(),
          offset,
          name(),
          allocSize());
  }

  if (mem_flags_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    // Host visible buffer, map memory and memcpy
    Assert(mem_flags_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // fow now
    vmaCopyMemoryToAllocation(
      device().allocator(), src.data(), allocation_, offset, src.size());
  }
  else {
    device().execute(
      [&](const CommandBuffer& cb) { cb.copyDataToBuffer(*this, src); });
  }
}
NAMESPACE_END(eldr::vk::wr)
