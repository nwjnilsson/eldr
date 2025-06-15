#pragma once
#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

enum class MaterialPass : uint8_t { MainColor, Transparent, Other };

NAMESPACE_BEGIN(eldr)

struct MaterialInstance {
  vk::wr::Pipeline* pipeline;
  VkDescriptorSet   descriptor_set;
  MaterialPass      pass_type;
};

struct Material {
  MaterialInstance data;
};

struct GltfMetallicRoughness {
  EL_IMPORT_CORE_TYPES_SCALAR()
  vk::wr::Pipeline            opaque_pipeline;
  vk::wr::Pipeline            transparent_pipeline;
  vk::wr::DescriptorSetLayout material_layout;

  // Just an estimate of what will be needed
  static constexpr vk::PoolSizeRatio Sizes[3]{
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
  };

  struct MaterialConstants {
    Vector4f color_factors;
    Vector4f metal_rough_factors;
    // padding, we need it anyway for uniform buffers
    Vector4f extra[14];
  };

  struct Resources {
    const vk::wr::Image*                     color_texture;
    const vk::wr::Sampler*                   color_sampler;
    const vk::wr::Image*                     metal_rough_texture;
    const vk::wr::Sampler*                   metal_rough_sampler;
    const vk::wr::Buffer<MaterialConstants>* data_buffer;
    size_t                                   data_index;
  };

  MaterialInstance
  writeMaterial(const vk::wr::Device&                   device,
                MaterialPass                            pass,
                const GltfMetallicRoughness::Resources& resources,
                vk::DescriptorAllocator&                descriptor_allocator);

private:
  vk::DescriptorWriter writer;
};

NAMESPACE_END(eldr)
