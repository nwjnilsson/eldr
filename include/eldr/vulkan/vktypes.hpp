#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/math/glm.hpp>
#include <eldr/vulkan/vulkan.hpp>

// Misc Vulkan types
NAMESPACE_BEGIN(eldr::vk)
struct GpuDrawPushConstants {
  using Transform4f = CoreAliases<float>::Transform4f;
  Transform4f     world_transform;
  VkDeviceAddress vertex_buffer;
};

NAMESPACE_END(eldr::vk)
