#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/material.hpp>
#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/descriptorbuilder.hpp>
#include <eldr/vulkan/wrappers/pipelinebuilder.hpp>

namespace eldr::vk {
GltfMetallicRoughness::buildPipelines(VulkanEngine* engine)
{
  const VkPushConstantRange matrix_range{
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset     = 0,
    .size       = sizeof(GpuDrawPushConstants),
  };

  wr::DescriptorBuilder descriptor_builder{ engine->device() };
  descriptor_builder.addUniformBuffer<MaterialConstants>(buff, 0)
    .addCombinedImageSampler(sampler, view, 1)
    .addCombinedImageSampler(sampler, view, 2);
  DescriptorLayoutBuilder layout_builder;
  layout_builder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  layout_builder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  layout_builder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

  material_layout = layout_builder.build(
    engine->device_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

  VkDescriptorSetLayout layouts[] = { engine->_gpuSceneDataDescriptorLayout,
                                      material_layout };

  wr::PipelineBuilder pipeline_builder(engine->device());
  pipeline_builder.addDescriptorSetLayout(engine->sceneDescriptorLayout())
    .addDescriptorSetLayout(material_layout)
    .setShaders(engine->getShader("vert"), engine->getShader("frag"))
    .setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
    .setPolygonMode(VK_POLYGON_MODE_FILL)
    .setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
    .setMultisamplingNone()
    .disableBlending()
    .enableDepthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL)

    // render format
    .setColorAttachmentFormat(engine->draw_image_.imageFormat)
    .setDepthFormat(engine->depth_image_.imageFormat)

    // finally build the pipeline
    opaque_pipeline.pipeline = pipeline_builder.buildPipeline(engine->device_);

  // create the transparent variant
  pipeline_builder.enable_blending_additive();

  pipeline_builder.enable_depthtest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);

  transparent_pipeline.pipeline =
    pipeline_builder.buildPipeline(engine->device_);
}
} // namespace eldr::vk
