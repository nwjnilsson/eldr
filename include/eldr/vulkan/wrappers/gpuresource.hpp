#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {

class GpuResource {
protected:
  GpuResource(const Device& device, const std::string& name)
    : device_(device), name_(name)
  {
  }

public:
  // Since vmaDestroyImage/vmaDestroyBuffer also frees allocated memory,
  // the cleanup is handled in the derived class' destructor
  virtual ~GpuResource() = default;

protected:
  const Device& device_;

protected:
  std::string       name_;
  VmaAllocation     allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo alloc_info_{};
};
} // namespace eldr::vk::wr
