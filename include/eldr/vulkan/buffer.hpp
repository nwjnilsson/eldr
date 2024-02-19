#pragma once
#include <eldr/vulkan/command.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>

#include <glm/glm.hpp>

#include <array>

namespace eldr {
namespace vk {
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct BufferInfo {
  VkDeviceSize          size;
  VkBufferUsageFlags    usage;
  VkMemoryPropertyFlags properties;
};
// TODO: move definitions to cpp file (if this struct is to be used at all)
struct VkVertex {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 tex_coord;

  static VkVertexInputBindingDescription getBindingDescription()
  {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding   = 0;
    binding_description.stride    = sizeof(VkVertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
  }

  static std::array<VkVertexInputAttributeDescription, 3>
  getAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};
    attribute_descriptions[0].binding  = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset   = offsetof(VkVertex, pos);

    attribute_descriptions[1].binding  = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset   = offsetof(VkVertex, color);

    attribute_descriptions[2].binding  = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset   = offsetof(VkVertex, tex_coord);

    return attribute_descriptions;
  }
};
class Buffer {
public:
  Buffer();
  Buffer(const Device*, const BufferInfo&);
  Buffer(const Device*, const std::vector<VkVertex>&,
         CommandPool&); // Vertex buffer
  Buffer(const Device*, const std::vector<uint16_t>&,
         CommandPool&); // Index buffer
  Buffer(Buffer&&) = default;
  ~Buffer();

  Buffer& operator=(Buffer&&);

  VkDeviceSize          size() const { return size_; }
  const VkBuffer&       get() const { return buffer_; }
  const VkDeviceMemory& memory() const { return buffer_memory_; }
  void                  copyFrom(Buffer&, CommandPool&);

protected:
  const Device* device_;

  VkBuffer       buffer_;
  VkDeviceMemory buffer_memory_;
  VkDeviceSize   size_;
};
} // namespace vk
} // namespace eldr
