#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/common.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace eldr::vk {

struct GpuVertex {
  ELDR_IMPORT_CORE_TYPES();
  Vec3f   pos;
  Vec2f   uv; // texture coord
  Color3f color;

  bool operator==(const GpuVertex& other) const
  {
    return pos == other.pos && color == other.color && uv == other.uv;
  }
};
} // namespace eldr::vk
namespace std {
template <> struct hash<eldr::vk::GpuVertex> {
  ELDR_IMPORT_CORE_TYPES();
  size_t operator()(eldr::vk::GpuVertex const& vertex) const;
};
} // namespace std
