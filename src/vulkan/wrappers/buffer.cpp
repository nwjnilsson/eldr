#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// BufferImpl
//------------------------------------------------------------------------------
template <typename T>
class Buffer<T>::BufferImpl : public GpuResourceAllocation {
public:
  BufferImpl(const Device& device, const VkBufferCreateInfo& buffer_ci,
             const VmaAllocationCreateInfo& alloc_ci);
  ~BufferImpl();
  VkBuffer buffer_{ VK_NULL_HANDLE };
};

template <typename T>
Buffer<T>::BufferImpl::BufferImpl(const Device&                  device,
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

template <typename T> Buffer<T>::BufferImpl::~BufferImpl()
{
  vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
}

//------------------------------------------------------------------------------
// Buffer
//------------------------------------------------------------------------------
template <typename T>
Buffer<T>::Buffer(const Device& device, std::string_view name,
                  size_t element_count, VkBufferUsageFlags buffer_usage,
                  VmaMemoryUsage           memory_usage,
                  VmaAllocationCreateFlags alloc_flags)
  : size_(element_count), size_bytes_(size_ * sizeof(T))
{
  assert(size_bytes_ > 0);
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

template <typename T>
Buffer<T>::Buffer(const Device& device, std::string_view name,
                  std::span<T> data, VkBufferUsageFlags buffer_usage,
                  VmaMemoryUsage           memory_usage,
                  VmaAllocationCreateFlags alloc_flags)
  : Buffer(device, name, data.size(), buffer_usage, memory_usage, alloc_flags)
{
  uploadData(data);
}

template <typename T> void Buffer<T>::uploadData(std::span<T> data) const
{
  assert(data.size() > 0);
  assert(data.size_bytes() <= size_bytes_);
  assert(buffer_data_->alloc_info_.pMappedData != nullptr);
  std::memcpy(buffer_data_->alloc_info_.pMappedData, data.data(),
              data.size_bytes());
}

template <typename T> VkBuffer Buffer<T>::get() const
{
  return buffer_data_->buffer_;
}
} // namespace eldr::vk::wr
