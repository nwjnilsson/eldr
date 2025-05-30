#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

#include <span>

namespace eldr::vk::wr {

// TODO: untemplatize this wrapper class? The template does provide some type
// safety and indicates purpose, but not sure if that will be worth it when
// integrating with the render graph, which generalizes the buffer anyway. Can
// have a templatized constructor maybe, and a templatized upload function.
// Should probably support uploading to a specific section of the buffer, and
// not just overwriting from the beginning of the mapped data
template <typename T> class Buffer {
private:
  class BufferImpl : public GpuResourceAllocation {
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

    ~BufferImpl()
    {
      vmaDestroyBuffer(device_.allocator(), buffer_, allocation_);
    }

    VkBuffer buffer_{ VK_NULL_HANDLE };
  };

public:
  Buffer() = default;
  Buffer(const Device&      device,
         const std::string& name,
         size_t             element_count,
         BufferUsageFlags   buffer_usage,
         HostAccessFlags    host_access,
         MemoryUsage        mem_usage = MemoryUsage::Auto)
    : size_(element_count), size_bytes_(size_ * sizeof(T))
  {
    Assert(size_ > 0);
    const VkBufferCreateInfo buffer_ci{
      .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext                 = {},
      .flags                 = {},
      .size                  = size_bytes_,
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
  }

  Buffer(const Device&      device,
         const std::string& name,
         std::span<const T> data,
         BufferUsageFlags   buffer_usage,
         HostAccessFlags    host_access,
         MemoryUsage        mem_usage = MemoryUsage::Auto)
    : Buffer(device, name, data.size(), buffer_usage, host_access, mem_usage)
  {
    uploadData(data);
  }

  // VkDeviceSize    size() const { return size_; }
  [[nodiscard]] const char* name() const { return d_->alloc_info_.pName; }
  [[nodiscard]] VkBuffer    vk() const { return d_->buffer_; }
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
      .buffer = d_->buffer_
    };
    return vkGetBufferDeviceAddress(d_->device_.logical(), &address_info);
  }

  /// @brief Copies `data` to the mapped GPU memory.
  /// @tparam T The type of data to upload.
  /// @param data The span of data to upload to the GPU buffer.
  void uploadData(std::span<const T> data) const
  {
    Assert(data.size() > 0);
    Assert(data.size_bytes() <= size_bytes_);

    if (not(d_->mem_flags_ & MemoryProperty::HostVisible)) {
      Throw("GPU only buffer uploads not implemented yet");
    }

    constexpr MemoryPropertyFlags valid_buffer_flags{
      MemoryProperty::HostCoherent
    };

    Assert(d_->mem_flags_ & valid_buffer_flags);
    void* dst{ nullptr };
    vmaMapMemory(d_->device_.allocator(), d_->allocation_, &dst);
    Assert(dst);
    std::memcpy(dst, data.data(), data.size_bytes());
    vmaUnmapMemory(d_->device_.allocator(), d_->allocation_);

    // if not HOST_COHERENT, a flush is needed using vmaInvalidateAllocation() /
    // vmaFlushAllocation()
  }
  // void copyFromBuffer(const Buffer&, const CommandPool&);

private:
  std::shared_ptr<BufferImpl> d_;
  size_t                      size_{ 0 };
  size_t                      size_bytes_{ 0 };
};
} // namespace eldr::vk::wr
