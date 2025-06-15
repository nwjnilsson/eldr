#include <eldr/vulkan/descriptorallocator.hpp>
#include <eldr/vulkan/descriptorsetlayoutbuilder.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

NAMESPACE_BEGIN(eldr)
MaterialInstance GltfMetallicRoughness::writeMaterial(
  const vk::wr::Device&                   device,
  MaterialPass                            pass,
  const GltfMetallicRoughness::Resources& resources,
  vk::DescriptorAllocator&                descriptor_allocator)
{
  MaterialInstance mat_data;
  mat_data.pass_type = pass;
  if (pass == MaterialPass::Transparent) {
    mat_data.pipeline = &transparent_pipeline;
  }
  else {
    mat_data.pipeline = &opaque_pipeline;
  }

  mat_data.descriptor_set =
    descriptor_allocator.allocate(device, material_layout);

  writer.reset();
  writer.writeUniformBuffer(0, *resources.data_buffer, resources.data_index);
  writer.writeCombinedImageSampler(1,
                                   *resources.color_texture,
                                   *resources.color_sampler,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  writer.writeCombinedImageSampler(2,
                                   *resources.metal_rough_texture,
                                   *resources.metal_rough_sampler,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  writer.updateSet(device, mat_data.descriptor_set);

  return mat_data;
}
NAMESPACE_END(eldr)
