#include <eldr/core/logger.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {

Buffer::Buffer(const Device& device, const BufferResource& buffer_resource,
               const VmaAllocationCreateInfo& alloc_ci)
  : PhysicalResource(device)
{
  if (buffer_resource.data_size_ == 0) {
    return;
  }

  VkBufferCreateInfo buffer_ci{};
  buffer_ci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_ci.size        = buffer_resource.data_size_;
  buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  switch (buffer_resource.usage_) {
    case BufferUsage::index_buffer:
      buffer_ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
      break;
    case BufferUsage::vertex_buffer:
      buffer_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      break;
    default:
      assert(false);
  }

  if (const VkResult result =
        vmaCreateBuffer(device_.allocator(), &buffer_ci, &alloc_ci, &buffer_,
                        &allocation_, &alloc_info_);
      result != VK_SUCCESS)
    ThrowVk(result, "Failed to create buffer!");

  // TODO: improve naming
  vmaSetAllocationName(device_.allocator(), allocation_, "render graph buffer");

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device_.logical(), buffer_, &mem_requirements);
}

Buffer::~Buffer()
{
  vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
}

void Buffer::copyFromBuffer(const Buffer&      other,
                            const CommandPool& command_pool)
{
  CommandBuffer cb(device_, command_pool);
  cb.begin();
  VkBufferCopy copy_region{}; // There are optional offsets in this struct
  copy_region.size = alloc_info_.size;
  vkCmdCopyBuffer(cb.get(), other.buffer_, buffer_, 1, &copy_region);
  cb.submit();
}

void Buffer::uploadData(const void* data, size_t size)
{
  assert(alloc_info_.pMappedData != nullptr);
  std::memcpy(alloc_info_.pMappedData, data, size);
}

} // namespace eldr::vk::wr
