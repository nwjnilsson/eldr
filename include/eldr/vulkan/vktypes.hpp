#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr::vk {

struct GpuResourceAllocation {
  const wr::Device& device_;
  VmaAllocation     allocation_{ VK_NULL_HANDLE };
  VmaAllocationInfo alloc_info_{};
};

struct GpuDrawPushConstants {
  using Mat4f = CoreAliases<Float>::Mat4f;
  Mat4f           world_matrix;
  VkDeviceAddress vertex_buffer;
};

struct GpuVertex {
  ELDR_IMPORT_CORE_TYPES();
  Point3f pos;
  float   uv_x;
  Vec3f   normal;
  float   uv_y;
  Color4f color;
};

} // namespace eldr::vk
namespace std {
template <> struct hash<eldr::vk::GpuVertex> {
  ELDR_IMPORT_CORE_TYPES();
  size_t operator()(eldr::vk::GpuVertex const& vertex) const;
};
} // namespace std
