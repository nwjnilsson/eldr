#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
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
  ELDR_IMPORT_CORE_TYPES()
  vk::wr::Pipeline            opaque_pipeline;
  vk::wr::Pipeline            transparent_pipeline;
  vk::wr::DescriptorSetLayout material_layout;

  struct MaterialConstants {
    Vec4f color_factors;
    Vec4f metal_rough_factors;
    // padding, we need it anyway for uniform buffers
    Vec4f extra[14];
  };

  struct MaterialResources {
    vk::wr::Texture* color_texture;
    vk::wr::Sampler* color_sampler;
    vk::wr::Texture* metal_rough_texture;
    vk::wr::Sampler* metal_rough_sampler;
    vk::wr::Buffer   data_buffer;
    size_t           data_buffer_offset;
  };

  MaterialInstance writeMaterial(const vk::wr::Device&    device,
                                 MaterialPass             pass,
                                 const MaterialResources& resources,
                                 vk::DescriptorAllocator& descriptor_allocator);

private:
  vk::DescriptorWriter writer;
};

} // namespace eldr
