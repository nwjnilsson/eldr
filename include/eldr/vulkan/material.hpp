#pragma once
#include <eldr/math/vector.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

enum class MaterialPass : uint8_t { MainColor, Transparent, Other };

namespace eldr {
struct MaterialInstance {
  vk::wr::Pipeline* pipeline;
  VkDescriptorSet   descriptor_set;
  MaterialPass      pass_type;
};

struct Material {
  MaterialInstance data;
};

struct GltfMetallicRoughness {
  // TODO: import vulkan types with some namespace aliasing
  using Float = float;
  EL_IMPORT_CORE_TYPES()
  vk::wr::Pipeline            opaque_pipeline;
  vk::wr::Pipeline            transparent_pipeline;
  vk::wr::DescriptorSetLayout material_layout;

  struct MaterialConstants {
    Vector4f color_factors;
    Vector4f metal_rough_factors;
    // padding, we need it anyway for uniform buffers
    Vector4f extra[14];
  };

  struct MaterialResources {
    const vk::wr::Image*                     color_texture;
    const vk::wr::Sampler*                   color_sampler;
    const vk::wr::Image*                     metal_rough_texture;
    const vk::wr::Sampler*                   metal_rough_sampler;
    const vk::wr::Buffer<MaterialConstants>* data_buffer;
    size_t                                   data_index;
  };

  MaterialInstance writeMaterial(const vk::wr::Device&    device,
                                 MaterialPass             pass,
                                 const MaterialResources& resources,
                                 vk::DescriptorAllocator& descriptor_allocator);

private:
  vk::DescriptorWriter writer;
};

} // namespace eldr
