#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/vulkan.hpp>

// Misc Vulkan types
namespace eldr::vk {
struct GpuResourceAllocation {
  const wr::Device&     device_;
  VmaAllocation         allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo     alloc_info_;
  VkMemoryPropertyFlags mem_flags_;
};

struct GpuDrawPushConstants {
  using Mat4f = CoreAliases<Float>::Mat4f;
  Mat4f           world_matrix;
  VkDeviceAddress vertex_buffer;
};

} // namespace eldr::vk
