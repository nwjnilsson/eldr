#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/pipelinebuilder.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

namespace eldr::vk::wr {
void PipelineBuilder::clear()
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

  pipeline_layout_ = {};

  depth_stencil_ = {};
  depth_stencil_.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

  render_info_       = {};
  render_info_.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

  shader_stages_.clear();
}

PipelineBuilder& PipelineBuilder::setInputTopology(VkPrimitiveTopology topology)
{
  input_assembly_.topology               = topology;
  input_assembly_.primitiveRestartEnable = VK_FALSE;
  return *this;
}

PipelineBuilder& PipelineBuilder::setShaders(const Shader& vertex_shader,
                                             const Shader& fragment_shader)
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

GraphicsPipeline PipelineBuilder::build(const Device&    device,
                                        std::string_view name)
{
  // Pipeline layout
  const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
    .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext          = nullptr,
    .flags          = 0,
    .setLayoutCount = static_cast<uint32_t>(descriptor_layouts_.size()),
    .pSetLayouts    = descriptor_layouts_.data(),
    .pushConstantRangeCount =
      static_cast<uint32_t>(push_constant_ranges_.size()),
    .pPushConstantRanges = push_constant_ranges_.data(),
  };

  // Pipeline
  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  const VkPipelineViewportStateCreateInfo viewport_state = {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext         = nullptr,
    .flags         = 0,
    .viewportCount = 1,
    .pViewports    = nullptr, // dynamic viewport
    .scissorCount  = 1,
    .pScissors     = nullptr, // dynamic scissor
  };

  // dummy color blending
  const VkPipelineColorBlendStateCreateInfo color_blending{
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .blendConstants  = {},
    .logicOpEnable   = VK_FALSE,
    .logicOp         = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments    = &color_blend_attachment_,
  };

  const std::vector<VkDynamicState> dynamic_states = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
  };

  const VkPipelineDynamicStateCreateInfo dynamic_state = {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext             = nullptr,
    .flags             = 0,
    .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
    .pDynamicStates    = dynamic_states.data()
  };

  const VkGraphicsPipelineCreateInfo pipeline_ci = {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = nullptr,
    .flags               = 0,
    .stageCount          = static_cast<std::uint32_t>(shader_stages_.size()),
    .pStages             = shader_stages_.data(),
    .pVertexInputState   = &vertex_input_info,
    .pInputAssemblyState = &input_assembly_,
    .pTessellationState  = nullptr,
    .pViewportState      = &viewport_state,
    .pRasterizationState = &rasterizer_,
    .pMultisampleState   = &multisampling_,
    .pDepthStencilState  = &depth_stencil_,
    .pColorBlendState    = &color_blending,
    .pDynamicState       = &dynamic_state,
    .layout              = pipeline_layout_,
    .renderPass          = {}, // dynamic rendering
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
    .basePipelineIndex   = -1,
  };

  GraphicsPipeline pipeline{ device, name, pipeline_layout_ci, pipeline_ci };
  clear();
  return pipeline;
}
} // namespace eldr::vk::wr
