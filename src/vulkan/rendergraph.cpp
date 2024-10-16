#include <eldr/core/logger.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <vulkan/vulkan_core.h>

namespace eldr::vk {

PhysicalResource::~PhysicalResource()
{
  if (allocation_ != VK_NULL_HANDLE)
    vkFreeMemory(device_.logical(), allocation_, nullptr);
}

void RenderGraph::buildRenderPass(const GraphicsStage*   stage,
                                  PhysicalGraphicsStage& physical) const
{
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference>   color_refs;
  std::vector<VkAttachmentReference>   color_resolve_refs;
  std::vector<VkAttachmentReference>   depth_refs;

  // Build vulkan attachments. For every texture resource that stage writes to,
  // we create a corresponding VkAttachmentDescription and attach it to the
  // render pass.
  // TODO(GH-203): Support multisampled attachments.
  // TODO: Use range-based for loop initialization statements when we switch to
  // C++ 20.
  for (std::size_t i = 0; i < stage->writes_.size(); i++) {
    const auto* resource = stage->writes_[i];
    const auto* texture  = resource->as<TextureResource>();
    if (texture == nullptr) {
      continue;
    }

    VkAttachmentDescription attachment{};
    attachment.flags   = 0;
    attachment.format  = texture->format_;
    attachment.samples = device_.getMaxMSAASampleCount();
    attachment.loadOp  = stage->clears_screen_ ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                               : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

    switch (texture->usage_) {
      case TextureUsage::back_buffer:
        attachment.samples = VK_SAMPLE_COUNT_1_BIT; // use only one sample when
                                                    // resolving to back buffer
        if (!stage->clears_screen_) {
          attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        color_resolve_refs.push_back(
          { static_cast<std::uint32_t>(i),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        break;
      case TextureUsage::offscreen_buffer:
        // currently the same as default/"normal" case
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(
          { static_cast<std::uint32_t>(i), attachment.finalLayout });
        break;
      case TextureUsage::depth_stencil_buffer:
        attachment.finalLayout =
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        depth_refs.push_back(
          { static_cast<std::uint32_t>(i),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
        break;
      default:
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(
          { static_cast<std::uint32_t>(i), attachment.finalLayout });
        break;
    }
    attachments.push_back(attachment);
  }

  physical.render_pass_ = std::make_unique<wr::RenderPass>(
    device_, attachments, color_refs, color_resolve_refs, depth_refs);
}

void RenderGraph::buildGraphicsPipeline(const GraphicsStage*   stage,
                                        PhysicalGraphicsStage& physical) const
{
  // TODO: Put this in pipeline wrapper. Need to pass various bindings etc
  // Passing the stage and doing everything in the pipeline constructor should
  // be possible too


  // Build buffer and vertex layout bindings. For every buffer resource that
  // stage reads from, we create a corresponding attribute binding and vertex
  // binding description.
  std::vector<VkVertexInputAttributeDescription> attribute_bindings;
  std::vector<VkVertexInputBindingDescription>   vertex_bindings;
  for (const auto* resource : stage->reads_) {
    const auto* buffer_resource = resource->as<BufferResource>();
    if (buffer_resource == nullptr) {
      continue;
    }

    // Don't mess with index buffers here.
    if (buffer_resource->usage_ == BufferUsage::index_buffer) {
      continue;
    }

    // We use std::unordered_map::at() here to ensure that a binding value
    // exists for buffer_resource.
    const std::uint32_t binding = stage->buffer_bindings_.at(buffer_resource);
    for (auto attribute_binding : buffer_resource->vertex_attributes_) {
      attribute_binding.binding = binding;
      attribute_bindings.push_back(attribute_binding);
    }

    vertex_bindings.push_back({
      .binding   = binding,
      .stride    = static_cast<std::uint32_t>(buffer_resource->element_size_),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    });
  }

  const VkPipelineVertexInputStateCreateInfo vertex_input = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount =
      static_cast<std::uint32_t>(vertex_bindings.size()),
    .pVertexBindingDescriptions = vertex_bindings.data(),
    .vertexAttributeDescriptionCount =
      static_cast<std::uint32_t>(attribute_bindings.size()),
    .pVertexAttributeDescriptions = attribute_bindings.data(),
  };

  const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
    .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  const VkPipelineDepthStencilStateCreateInfo depth_stencil = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable       = stage->depth_test_ ? VK_TRUE : VK_FALSE,
    .depthWriteEnable      = stage->depth_write_ ? VK_TRUE : VK_FALSE,
    .depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable     = VK_FALSE,
    .front                 = {},
    .back                  = {},
    .minDepthBounds        = 0.0f, // optional
    .maxDepthBounds        = 1.0f, // optional
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
    .lineWidth               = 1.0f,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0,
    .depthBiasClamp          = 0,
    .depthBiasSlopeFactor    = 0,
  };

  const VkPipelineMultisampleStateCreateInfo multisample_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .rasterizationSamples  = device_.getMaxMSAASampleCount(),
    .sampleShadingEnable   = VK_TRUE,
    .minSampleShading      = 0.2f,
    .pSampleMask           = nullptr,  // Optional
    .alphaToCoverageEnable = VK_FALSE, // Optional
    .alphaToOneEnable      = VK_FALSE, // Optional
  };

  auto blend_attachment = stage->blend_attachment_;
  blend_attachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  // TODO: the lines below may be needed
  // blend_attachment.blendEnable         = VK_TRUE;
  // blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // blend_attachment.dstColorBlendFactor =
  //  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
  // blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

  const VkPipelineColorBlendStateCreateInfo blend_state = {
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .attachmentCount = 1,
    .pAttachments    = &blend_attachment,
    .logicOpEnable   = VK_FALSE,
    .logicOp         = VK_LOGIC_OP_COPY,           // Optional
    .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f }, // Optional
  };

  const VkRect2D scissor{
    .offset = { 0, 0 },
    .extent = swapchain_.extent(),
  };

  const VkViewport viewport{
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = static_cast<float>(swapchain_.extent().width),
    .height   = static_cast<float>(swapchain_.extent().height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
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

  const VkGraphicsPipelineCreateInfo pipeline_ci = {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = nullptr,
    .flags               = 0,
    .stageCount          = static_cast<std::uint32_t>(stage->shaders_.size()),
    .pStages             = stage->shaders_.data(),
    .pVertexInputState   = &vertex_input,
    .pInputAssemblyState = &input_assembly,
    .pTessellationState  = nullptr,
    .pViewportState      = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState   = &multisample_state,
    .pDepthStencilState  = &depth_stencil,
    .pColorBlendState    = &blend_state,
    .pDynamicState       = nullptr,
    .layout              = physical.pipeline_->layout(),
    .renderPass          = physical.render_pass_->get(),
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
    .basePipelineIndex   = -1,
  };

  // TODO: Pipeline caching (basically load the render graph from a file)
  if (const auto result =
        vkCreateGraphicsPipelines(device_.logical(), nullptr, 1, &pipeline_ci,
                                  nullptr, &physical.pipeline_);
      result != VK_SUCCESS) {
    throw VulkanException(
      "Error: vkCreateGraphicsPipelines failed for pipeline " + stage->name() +
        " !",
      result);
  }
  physical.pipeline_ = std::make_unique<wr::Pipeline>(device_);
}

void RenderGraph::compile(const RenderResource* target)
{
  std::vector<RenderStage*> stage_set{}; // for topology sorting
  for (auto& stage : stages_) {
    stage_set.push_back(stage.get());
  }

  std::vector<size_t> sublist{}; // indices to stages with no read dependencies
  // Initialize topology sort
  for (size_t i = 0; i < stage_set.size(); ++i) {
    if (stage_set[i]->reads_.empty())
      sublist.push_back(i);
  }
  // Topology sort
  while (!sublist.empty()) {
    stage_stack_.push_back({});
    for (size_t i : sublist)
      stage_stack_.back().push_back(stage_set[i]);
    for (size_t i : sublist)
      stage_set.erase(stage_set.begin() + i);
    sublist.clear();
    for (size_t i = 0; i < stage_set.size(); ++i) {
      if (stage_set[i]->reads_.empty())
        sublist.push_back(i);
    }
  }
  if (!stage_set.empty()) {
    ThrowVk("Render Graph contains cyclic dependencies!");
  }
  else {
    // TODO: remove when not needed anymore, or change to trace
#ifdef _DEBUG
    std::string stage_order = "{";
    for (auto& sublist : stage_stack_) {
      stage_order += "{";
      for (auto stage : sublist) {
        stage_order += ", " + stage->name();
      }
      stage_order += "}";
    }
    stage_order += "}";
    log_->debug("Proposed stage order {}", stage_order);
#endif
  }

  // std::unordered_map<const RenderResource*, std::vector<RenderStage*>>
  // writers; for (auto& stage : stages_) {
  //   for (const auto* resource : stage->writes_) {
  //     writers[resource].push_back(stage.get());
  //   }
  // }

  log_->trace("Allocating physical resource for buffers:");

  // for (auto& buffer_resource : buffer_resources_) {
  //   log_->trace("   - {}", buffer_resource->name_);
  //   buffer_resource->physical_ = std::make_shared<Buffer>(device_);
  // }

  log_->trace("Allocating physical resource for texture:");
  for (auto& texture_resource : texture_resources_) {
    log_->trace(" - {}", texture_resource->name());
    const wr::ImageInfo texture_info = {
      .extent      = swapchain_.extent(),
      .format      = texture_resource->format_,
      .tiling      = VK_IMAGE_TILING_OPTIMAL,
      .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT,
      .memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
      .num_samples  = texture_resource->num_samples_,
      .mip_levels   = texture_resource->mip_levels_
    };
    auto physical               = std::make_shared<PhysicalImage>(device_);
    texture_resource->physical_ = physical;
    physical->image_ = std::make_unique<wr::Image>(device_, texture_info);
    physical->image_view_ =
      std::make_unique<wr::ImageView>(device_, *physical->image_, texture_info);
  }

  for (auto substages : stage_stack_) {
    for (auto* stage : substages) {
      if (auto* graphics_stage = stage->as<GraphicsStage>()) {
        auto  physical_ptr = std::make_unique<PhysicalGraphicsStage>(device_);
        auto& physical     = *physical_ptr;
        graphics_stage->physical_ = std::move(physical_ptr);

        buildRenderPass(graphics_stage, physical);
        buildPipelineLayout(graphics_stage, physical);
        buildGraphicsPipeline(graphics_stage, physical);

        // If we write to at least one texture, we need to make framebuffers.
        if (!stage->writes_.empty()) {
          // For every texture that this stage writes to, we need to attach it
          // to the framebuffer.
          std::vector<const PhysicalBackBuffer*> back_buffers;
          std::vector<const PhysicalImage*>      images;
          for (const auto* resource : stage->writes_) {
            if (const auto* texture = resource->as<TextureResource>()) {
              const auto& physical_texture = *texture->physical_;
              if (const auto* back_buffer =
                    physical_texture.as<PhysicalBackBuffer>()) {
                back_buffers.push_back(back_buffer);
              }
              else if (const auto* image =
                         physical_texture.as<PhysicalImage>()) {
                images.push_back(image);
              }
            }
          }

          std::vector<VkImageView> image_views;
          image_views.reserve(back_buffers.size() + images.size());
          for (auto* const img_view : swapchain_.imageViews()) {
            std::fill_n(std::back_inserter(image_views), back_buffers.size(),
                        img_view);
            for (const auto* image : images) {
              image_views.push_back(image->image_view_->get());
            }
            physical.framebuffers_.emplace_back(device_, *physical.render_pass_,
                                                image_views, swapchain_);
            image_views.clear();
          }
        }
      }
    }
  }
}

void RenderGraph::render(uint32_t image_index, const wr::CommandBuffer& cb) {}

} // namespace eldr::vk
