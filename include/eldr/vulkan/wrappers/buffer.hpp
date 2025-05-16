#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <span>

namespace eldr::vk::wr {
template <typename T> class Buffer {
private:
  class BufferImpl : public GpuResourceAllocation {
  public:
    BufferImpl(const Device&                  device,
               const VkBufferCreateInfo&      buffer_ci,
               const VmaAllocationCreateInfo& alloc_ci)
      : GpuResourceAllocation(device)
    {
      if (const VkResult result = vmaCreateBuffer(device_.allocator(),
                                                  &buffer_ci,
                                                  &alloc_ci,
                                                  &buffer_,
                                                  &allocation_,
                                                  &alloc_info_);
          result != VK_SUCCESS)
        ThrowVk(result, "vmaCreateBuffer(): ");
    }
    ~BufferImpl()
    {
      vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
    }

    VkBuffer buffer_{ VK_NULL_HANDLE };
  };

public:
  Buffer() = default;
  Buffer(
    const Device&            device,
    std::string_view         name,
    size_t                   element_count,
    VkBufferUsageFlags       buffer_usage,
    VmaMemoryUsage           memory_usage,
    VmaAllocationCreateFlags alloc_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT)
    : size_(element_count), size_bytes_(size_ * sizeof(T))
  {
    assert(size_ > 0);
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
    b_data_ = std::make_shared<BufferImpl>(device, buffer_ci, alloc_ci);
    vmaSetAllocationName(device.allocator(),
                         b_data_->allocation_,
                         fmt::format("{} allocation", name).c_str());
  }

  Buffer(
    const Device&            device,
    std::string_view         name,
    std::span<const T>       data,
    VkBufferUsageFlags       buffer_usage,
    VmaMemoryUsage           memory_usage,
    VmaAllocationCreateFlags alloc_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT)
    : Buffer(device, name, data.size(), buffer_usage, memory_usage, alloc_flags)
  {
    uploadData(data);
  }

  // VkDeviceSize    size() const { return size_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkBuffer           get() const { return b_data_->buffer_; }
  /// @return size_t The number of elements in the buffer.
  [[nodiscard]] size_t size() const { return size_; }

  /// @brief Returns the size, in bytes, requested upon buffer creation.
  /// The real size of the VMA allocation may be greater. See VMA allocation
  /// creation docs for more info.
  /// @return VkDeviceSize The size requested upon buffer creation, in bytes.
  [[nodiscard]] VkDeviceSize size_bytes() const { return size_bytes_; }

  /// @brief Returns whether the buffer is empty or not.
  /// @return bool `true` if size_ == 0.
  [[nodiscard]] bool empty() const { return size_ == 0; }

  /// @brief Returns a VkDeviceAddress for this buffer
  [[nodiscard]] VkDeviceAddress getDeviceAddress() const
  {
    VkBufferDeviceAddressInfo address_info{
      .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .pNext  = {},
      .buffer = b_data_->buffer_
    };
    return vkGetBufferDeviceAddress(b_data_->device_.logical(), &address_info);
  }

  /// @brief Copies `data` to the mapped GPU memory.
  /// @tparam T The type of data to upload.
  /// @param data The span of data to upload to the GPU buffer.
  void uploadData(std::span<const T> data) const
  {
    assert(b_data_->alloc_info_.pMappedData != nullptr);
    assert(data.size() > 0);
    assert(data.size_bytes() <= size_bytes_);
    std::memcpy(
      b_data_->alloc_info_.pMappedData, data.data(), data.size_bytes());
  }
  // void copyFromBuffer(const GpuBuffer&, const CommandPool&);

private:
  std::string                 name_;
  std::shared_ptr<BufferImpl> b_data_;
  size_t                      size_{ 0 };
  // TODO: not sure if still needed
  VkDeviceSize size_bytes_{ 0 };
};
} // namespace eldr::vk::wr
