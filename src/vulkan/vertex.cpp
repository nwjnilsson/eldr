#include <eldr/core/hash.hpp>
#include <eldr/vulkan/vertex.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
size_t
hash<eldr::vk::GpuVertex>::operator()(eldr::vk::GpuVertex const& vertex) const
{
  size_t value{ hash<Point3f>()(vertex.pos) };
  value = eldr::hashCombine(value, hash<Vec2f>()(vertex.uv));
  value = eldr::hashCombine(value, hash<Color4f>()(vertex.color));
  value = eldr::hashCombine(value, hash<Vec3f>()(vertex.normal));
  return value;
};
} // namespace std
