#include <eldr/core/logger.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vertex.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk::wr {
Pipeline::Pipeline(
  const Device& device, const Swapchain& swapchain, const GraphicsStage& stage,
  const PhysicalGraphicsStage&                          physical_stage,
  const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions,
  const std::vector<VkVertexInputBindingDescription>&   vertex_bindings)
  : device_(device)
{
  const VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount =
      static_cast<std::uint32_t>(vertex_bindings.size()),
    .pVertexBindingDescriptions = vertex_bindings.data(),
    .vertexAttributeDescriptionCount =
      static_cast<std::uint32_t>(attribute_descriptions.size()),
    .pVertexAttributeDescriptions = attribute_descriptions.data(),
  };

  const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  const VkViewport viewport{
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = static_cast<float>(swapchain.extent().width),
    .height   = static_cast<float>(swapchain.extent().height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  const VkRect2D scissor{
    .offset = { 0, 0 },
    .extent = swapchain.extent(),
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

  const VkPipelineViewportStateCreateInfo viewport_state = {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext         = nullptr,
    .flags         = 0,
    .viewportCount = 1,
    .pViewports    = &viewport,
    .scissorCount  = 1,
    .pScissors     = &scissor,
  };

  const VkPipelineRasterizationStateCreateInfo rasterization_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthClampEnable        = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode             = VK_POLYGON_MODE_FILL,
    .cullMode                = VK_CULL_MODE_BACK_BIT,
    .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0,
    .depthBiasClamp          = 0,
    .depthBiasSlopeFactor    = 0,
    .lineWidth               = 1.0f,
  };

  const VkPipelineMultisampleStateCreateInfo multisample_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .rasterizationSamples  = device_.maxMsaaSampleCount(),
    .sampleShadingEnable   = VK_TRUE,
    .minSampleShading      = 0.2f,
    .pSampleMask           = nullptr,  // Optional
    .alphaToCoverageEnable = VK_FALSE, // Optional
    .alphaToOneEnable      = VK_FALSE, // Optional
  };

  const VkPipelineDepthStencilStateCreateInfo depth_stencil = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable       = stage.depth_test_ ? VK_TRUE : VK_FALSE,
    .depthWriteEnable      = stage.depth_write_ ? VK_TRUE : VK_FALSE,
    .depthCompareOp        = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable     = VK_FALSE,
    .front                 = {},
    .back                  = {},
    .minDepthBounds        = 0.0f, // optional
    .maxDepthBounds        = 1.0f, // optional
  };

  // TODO: may want to change
  // VkPipelineColorBlendAttachmentState color_blend_attachment{};
  // color_blend_attachment.colorWriteMask =
  //  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
  //  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  // color_blend_attachment.blendEnable         = VK_TRUE;
  // color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // color_blend_attachment.dstColorBlendFactor =
  //  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
  // color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

  auto blend_attachment = stage.blend_attachment_;
  blend_attachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  const VkPipelineColorBlendStateCreateInfo blend_state = {
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .logicOpEnable   = VK_FALSE,
    .logicOp         = VK_LOGIC_OP_COPY, // Optional
    .attachmentCount = 1,
    .pAttachments    = &blend_attachment,
    .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f }, // Optional
  };

  const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
    .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext          = nullptr,
    .flags          = 0,
    .setLayoutCount = static_cast<uint32_t>(stage.descriptor_layouts_.size()),
    .pSetLayouts    = stage.descriptor_layouts_.data(),
    .pushConstantRangeCount =
      static_cast<uint32_t>(stage.push_constant_ranges_.size()),
    .pPushConstantRanges = stage.push_constant_ranges_.data(),
  };

  if (const auto result = vkCreatePipelineLayout(
        device_.logical(), &pipeline_layout_ci, nullptr, &layout_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreatePipelineLayout(): failed for pipeline layout {}!",
            stage.name());
  }
  const VkGraphicsPipelineCreateInfo pipeline_ci = {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = nullptr,
    .flags               = 0,
    .stageCount          = static_cast<std::uint32_t>(stage.shaders_.size()),
    .pStages             = stage.shaders_.data(),
    .pVertexInputState   = &vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pTessellationState  = nullptr,
    .pViewportState      = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState   = &multisample_state,
    .pDepthStencilState  = &depth_stencil,
    .pColorBlendState    = &blend_state,
    .pDynamicState       = &dynamic_state,
    .layout              = layout_,
    .renderPass          = physical_stage.render_pass_->get(),
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
    .basePipelineIndex   = -1,
  };

  if (const auto result = vkCreateGraphicsPipelines(
        device_.logical(), nullptr, 1, &pipeline_ci, nullptr, &pipeline_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateGraphicsPipelines(): failed for pipeline {}!",
            stage.name());
  }
}

Pipeline::~Pipeline()
{
  if (pipeline_ != VK_NULL_HANDLE)
    vkDestroyPipeline(device_.logical(), pipeline_, nullptr);
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(device_.logical(), layout_, nullptr);
}

} // namespace eldr::vk::wr
