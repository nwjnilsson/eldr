#include <eldr/vulkan/vertex.hpp>

namespace std {
size_t
hash<eldr::vk::GpuVertex>::operator()(eldr::vk::GpuVertex const& vertex) const
{
  return ((hash<Vec3f>()(vertex.pos) ^ (hash<Vec2f>()(vertex.uv) << 1)) >> 1) ^
         (hash<Vec3f>()(vertex.color) << 1);
};
} // namespace std
