#include <eldr/core/logger.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/vertex.hpp>

#include <array>

namespace eldr {
namespace vk {
VkVertexInputBindingDescription Vertex::getBindingDescription()
{
  VkVertexInputBindingDescription binding_description{};
  binding_description.binding   = 0;
  binding_description.stride    = sizeof(Vertex);
  binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return binding_description;
}

std::array<VkVertexInputAttributeDescription, 3>
Vertex::getAttributeDescriptions()
{
  std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};
  attribute_descriptions[0].binding  = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[0].offset   = offsetof(Vertex, position);

  attribute_descriptions[1].binding  = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descriptions[1].offset   = offsetof(Vertex, color);

  attribute_descriptions[2].binding  = 0;
  attribute_descriptions[2].location = 2;
  attribute_descriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
  attribute_descriptions[2].offset   = offsetof(Vertex, tex_coord);

  return attribute_descriptions;
}

} // namespace vk
} // namespace eldr
namespace std {
size_t hash<eldr::vk::Vertex>::operator()(eldr::vk::Vertex const& vertex) const
{
  return ((hash<Vec3f>()(vertex.position) ^
           (hash<Vec3f>()(vertex.color) << 1)) >>
          1) ^
         (hash<Vec2f>()(vertex.tex_coord) << 1);
};
} // namespace std
