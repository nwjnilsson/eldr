#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

namespace eldr::vk {
void PipelineBuilder::reset()
{
  input_assembly_ = {};
  input_assembly_.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

  rasterizer_ = {};
  rasterizer_.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  color_blend_attachment_ = {};

  multisampling_ = {};
  multisampling_.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

  depth_stencil_ = {};
  depth_stencil_.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  render_info_       = {};
  render_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

  shader_stages_.clear();
}

PipelineBuilder& PipelineBuilder::addVertexAttribute(VkFormat format,
                                                     uint32_t offset)
{
  vertex_attributes_.push_back({
    .location = static_cast<uint32_t>(vertex_attributes_.size()),
    .binding  = 0,
    .format   = format,
    .offset   = offset,
  });
  return *this;
}

PipelineBuilder& PipelineBuilder::setInputTopology(VkPrimitiveTopology topology)
{
  input_assembly_.topology               = topology;
  input_assembly_.primitiveRestartEnable = VK_FALSE;
  return *this;
}

PipelineBuilder& PipelineBuilder::setShaders(const wr::Shader& vertex_shader,
                                             const wr::Shader& fragment_shader)
{
  shader_stages_.clear();
  shader_stages_.push_back({
    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext               = {},
    .flags               = {},
    .stage               = vertex_shader.stage(),
    .module              = vertex_shader.module(),
    .pName               = vertex_shader.entryPoint().c_str(),
    .pSpecializationInfo = {},
  });

  shader_stages_.push_back({
    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext               = {},
    .flags               = {},
    .stage               = fragment_shader.stage(),
    .module              = fragment_shader.module(),
    .pName               = fragment_shader.entryPoint().c_str(),
    .pSpecializationInfo = {},
  });
  return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(VkPolygonMode mode)
{
  rasterizer_.polygonMode = mode;
  rasterizer_.lineWidth   = 1.f;
  return *this;
}

PipelineBuilder& PipelineBuilder::setCullMode(VkCullModeFlags mode,
                                              VkFrontFace     front_face)
{
  rasterizer_.cullMode  = mode;
  rasterizer_.frontFace = front_face;
  return *this;
}

PipelineBuilder& PipelineBuilder::setMultisamplingNone()
{
  // multisampling defaulted to no multisampling (1 sample per pixel)
  multisampling_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling_.sampleShadingEnable  = VK_FALSE;
  multisampling_.minSampleShading     = 1.0f;
  multisampling_.pSampleMask          = nullptr;
  // no alpha to coverage either
  multisampling_.alphaToCoverageEnable = VK_FALSE;
  multisampling_.alphaToOneEnable      = VK_FALSE;
  return *this;
}

PipelineBuilder&
PipelineBuilder::setMultisampling(VkSampleCountFlagBits sample_count,
                                  float                 min_sample_shading)
{
  multisampling_.rasterizationSamples = sample_count;
  multisampling_.sampleShadingEnable  = VK_TRUE;
  multisampling_.minSampleShading     = min_sample_shading;
  multisampling_.pSampleMask          = nullptr;
  // no alpha to coverage
  multisampling_.alphaToCoverageEnable = VK_FALSE;
  multisampling_.alphaToOneEnable      = VK_FALSE;
  return *this;
}

PipelineBuilder& PipelineBuilder::disableBlending()
{
  // default write mask
  color_blend_attachment_.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  // no blending
  color_blend_attachment_.blendEnable = VK_FALSE;
  return *this;
}

PipelineBuilder& PipelineBuilder::enableBlendingAdditive()
{
  color_blend_attachment_.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment_.blendEnable         = VK_TRUE;
  color_blend_attachment_.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_.colorBlendOp        = VK_BLEND_OP_ADD;
  color_blend_attachment_.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment_.alphaBlendOp        = VK_BLEND_OP_ADD;
  return *this;
}

PipelineBuilder& PipelineBuilder::enableBlendingAlphaBlend()
{
  color_blend_attachment_.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment_.blendEnable         = VK_TRUE;
  color_blend_attachment_.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment_.dstColorBlendFactor =
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment_.colorBlendOp        = VK_BLEND_OP_ADD;
  color_blend_attachment_.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment_.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment_.alphaBlendOp        = VK_BLEND_OP_ADD;
  return *this;
}

PipelineBuilder& PipelineBuilder::setColorAttachmentFormat(VkFormat format)
{
  color_attachment_format_ = format;
  // connect the format to the renderInfo  structure
  render_info_.colorAttachmentCount    = 1;
  render_info_.pColorAttachmentFormats = &color_attachment_format_;
  return *this;
}

PipelineBuilder& PipelineBuilder::setDepthFormat(VkFormat format)
{
  render_info_.depthAttachmentFormat = format;
  return *this;
}

PipelineBuilder& PipelineBuilder::disableDepthtest()
{
  depth_stencil_.depthTestEnable       = VK_FALSE;
  depth_stencil_.depthWriteEnable      = VK_FALSE;
  depth_stencil_.depthCompareOp        = VK_COMPARE_OP_NEVER;
  depth_stencil_.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_.stencilTestEnable     = VK_FALSE;
  depth_stencil_.front                 = {};
  depth_stencil_.back                  = {};
  depth_stencil_.minDepthBounds        = 0.f;
  depth_stencil_.maxDepthBounds        = 1.f;
  return *this;
}

PipelineBuilder& PipelineBuilder::enableDepthtest(bool depth_write_enable,
                                                  VkCompareOp op)
{
  depth_stencil_.depthTestEnable       = VK_TRUE;
  depth_stencil_.depthWriteEnable      = depth_write_enable;
  depth_stencil_.depthCompareOp        = op;
  depth_stencil_.depthBoundsTestEnable = VK_FALSE;
  depth_stencil_.stencilTestEnable     = VK_FALSE;
  depth_stencil_.front                 = {};
  depth_stencil_.back                  = {};
  depth_stencil_.minDepthBounds        = 0.f;
  depth_stencil_.maxDepthBounds        = 1.f;
  return *this;
}

wr::Pipeline PipelineBuilder::build(const wr::Device&           device,
                                    std::string_view            name,
                                    VkPipelineLayoutCreateFlags layout_flags,
                                    VkPipelineCreateFlags       pipeline_flags)
{
  // Pipeline layout
  const VkPipelineLayoutCreateInfo pipeline_layout_ci{
    .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext          = nullptr,
    .flags          = layout_flags,
    .setLayoutCount = static_cast<uint32_t>(descriptor_layouts_.size()),
    .pSetLayouts    = descriptor_layouts_.data(),
    .pushConstantRangeCount =
      static_cast<uint32_t>(push_constant_ranges_.size()),
    .pPushConstantRanges = push_constant_ranges_.data(),
  };

  // optional, as vertex pulling can be used
  const VkPipelineVertexInputStateCreateInfo vertex_input_info{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = {},
    .flags = {},
    .vertexBindingDescriptionCount =
      static_cast<uint32_t>(vertex_bindings_.size()),
    .pVertexBindingDescriptions = vertex_bindings_.data(),
    .vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertex_attributes_.size()),
    .pVertexAttributeDescriptions = vertex_attributes_.data()
  };

  const VkPipelineViewportStateCreateInfo viewport_state{
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext         = nullptr,
    .flags         = 0,
    .viewportCount = 1,
    .pViewports    = nullptr, // dynamic viewport
    .scissorCount  = 1,
    .pScissors     = nullptr, // dynamic scissor
  };

  const VkPipelineColorBlendStateCreateInfo color_blending{
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .logicOpEnable   = VK_FALSE,
    .logicOp         = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments    = &color_blend_attachment_,
    .blendConstants  = {},
  };

  const std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR };

  const VkPipelineDynamicStateCreateInfo dynamic_state{
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext             = nullptr,
    .flags             = 0,
    .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
    .pDynamicStates    = dynamic_states.data()
  };

  VkGraphicsPipelineCreateInfo pipeline_ci{
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = &render_info_, // dynamic rendering info
    .flags               = pipeline_flags,
    .stageCount          = static_cast<uint32_t>(shader_stages_.size()),
    .pStages             = shader_stages_.data(),
    .pVertexInputState   = &vertex_input_info,
    .pInputAssemblyState = &input_assembly_,
    .pTessellationState  = {},
    .pViewportState      = &viewport_state,
    .pRasterizationState = &rasterizer_,
    .pMultisampleState   = &multisampling_,
    .pDepthStencilState  = &depth_stencil_,
    .pColorBlendState    = &color_blending,
    .pDynamicState       = &dynamic_state,
    .layout              = {},             // set later
    .renderPass          = VK_NULL_HANDLE, // dynamic rendering
    .subpass             = 0,
    .basePipelineHandle  = {},
    .basePipelineIndex   = -1,
  };

  return wr::Pipeline{ device, name, pipeline_layout_ci, pipeline_ci };
}
} // namespace eldr::vk
