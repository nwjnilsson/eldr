#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk {
MaterialInstance GltfMetallicRoughness::writeMaterial(
  const wr::Device& device, MaterialPass pass,
  const MaterialResources& resources, DescriptorAllocator& descriptor_allocator)
{
  MaterialInstance mat_data;
  mat_data.pass_type = pass;
  if (pass == MaterialPass::Transparent) {
    mat_data.pipeline = transparent_pipeline.get();
  }
  else {
    mat_data.pipeline = opaque_pipeline.get();
  }

  mat_data.material_set = descriptor_allocator.allocate(material_layout->get());

  writer.reset();
  writer.writeUniformBuffer<MaterialConstants>(0, *resources.data_buffer,
                                               resources.data_buffer_offset);
  writer.writeCombinedImageSampler(1, *resources.color_texture,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  writer.writeCombinedImageSampler(2, *resources.metal_rough_texture,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  writer.updateSet(device, mat_data.material_set);

  return mat_data;
}
} // namespace eldr::vk
