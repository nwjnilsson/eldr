#pragma once
#include <core/transform.hpp>
#include <vulkan/vulkan.hpp>

// Misc Vulkan types
NAMESPACE_BEGIN(eldr::vk)
struct GpuDrawPushConstants {
  using AffineTransform4f = CoreAliases<float>::AffineTransform4f;
  AffineTransform4f world_transform;
  AffineTransform4f model_transform;
  VkDeviceAddress   vertex_buffer;
};

NAMESPACE_END(eldr::vk)
