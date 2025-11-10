#pragma once
#include <core/transform.hpp>
#include <vulkan/vulkan.hpp>

// Misc Vulkan types
NAMESPACE_BEGIN(eldr::vk)
struct GpuDrawPushConstants {
  using Transform4f = CoreAliases<float>::Transform4f;
  Transform4f     world_transform;
  Transform4f     model_transform;
  VkDeviceAddress vertex_buffer;
};

NAMESPACE_END(eldr::vk)
