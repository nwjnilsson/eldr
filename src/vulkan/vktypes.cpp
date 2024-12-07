#include <eldr/core/hash.hpp>
#include <eldr/vulkan/vktypes.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
size_t
hash<eldr::vk::GpuVertex>::operator()(eldr::vk::GpuVertex const& vertex) const
{
  size_t value{ hash<Point3f>()(vertex.pos) };
  value = eldr::hashCombine(value, hash<float>()(vertex.uv_x));
  value = eldr::hashCombine(value, hash<Vec3f>()(vertex.normal));
  value = eldr::hashCombine(value, hash<float>()(vertex.uv_y));
  value = eldr::hashCombine(value, hash<Color4f>()(vertex.color));
  return value;
};
} // namespace std
