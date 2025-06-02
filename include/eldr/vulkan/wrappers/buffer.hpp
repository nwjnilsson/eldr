#pragma once
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/allocatedbuffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>

#include <span>

namespace eldr::vk::wr {

// Should probably support uploading to a specific section of the buffer, and
// not just overwriting from the beginning of the mapped data
template <typename T> class Buffer : public AllocatedBuffer {
public:
  Buffer() = default;
  Buffer(const Device&      device,
         const std::string& name,
         size_t             elem_count,
         BufferUsageFlags   buffer_usage,
         HostAccessFlags    host_access,
         MemoryUsage        mem_usage = MemoryUsage::Auto)
    : AllocatedBuffer(device,
                      name,
                      sizeof(T),
                      elem_count,
                      buffer_usage,
                      host_access,
                      mem_usage)
  {
    Assert(size() > 0);
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

  /// @brief Copies all contents of `data` into this buffer.
  /// @param data The span of data to upload to the GPU buffer.
  void uploadData(std::span<const T> src)
  {
    if (src.empty()) {
      return;
    }
    if (d_->mem_flags_.hasFlag(MemoryProperty::HostVisible)) {
      if (src.size_bytes() > sizeAlloc()) {
        Throw("Buffer size is too small.");
      }
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
  }
};
} // namespace eldr::vk::wr
