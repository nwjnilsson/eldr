#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/descriptorwriter.hpp>

namespace eldr::vk {

enum class MaterialPass : uint8_t { MainColor, Transparent, Other };

struct MaterialInstance {
  wr::Pipeline*   pipeline;
  VkDescriptorSet material_set;
  MaterialPass    pass_type;
};

struct GltfMetallicRoughness {
  ELDR_IMPORT_CORE_TYPES()
  std::unique_ptr<wr::Pipeline> opaque_pipeline;
  std::unique_ptr<wr::Pipeline> transparent_pipeline;

  std::unique_ptr<wr::DescriptorSetLayout> material_layout;

  struct MaterialConstants {
    Vec4f color_factors;
    Vec4f metal_rough_factors;
    // padding, we need it anyway for uniform buffers
    Vec4f extra[14];
  };

  struct MaterialResources {
    wr::GpuTexture*                color_texture;
    wr::GpuTexture*                metal_rough_texture;
    std::unique_ptr<wr::GpuBuffer> data_buffer;
    size_t                         data_buffer_offset;
  };

  MaterialInstance writeMaterial(const wr::Device& device, MaterialPass pass,
                                 const MaterialResources& resources,
                                 DescriptorAllocator&     descriptor_allocator);

private:
  DescriptorWriter writer;
};

} // namespace eldr::vk
