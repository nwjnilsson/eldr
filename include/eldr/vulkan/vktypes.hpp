#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/math/glm.hpp>
#include <eldr/vulkan/vulkan.hpp>

// Misc Vulkan types
NAMESPACE_BEGIN(eldr::vk)
struct GpuResourceAllocation {
  const wr::Device&     device_;
  VmaAllocation         allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo     alloc_info_;
  VkMemoryPropertyFlags mem_flags_;
};

struct GpuDrawPushConstants {
  using Transform4f = core::Aliases<float>::Transform4f;
  Transform4f     world_transform;
  VkDeviceAddress vertex_buffer;
};

NAMESPACE_END(eldr::vk)
