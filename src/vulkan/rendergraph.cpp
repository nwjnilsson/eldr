#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk {
PhysicalImage::PhysicalImage(const wr::Device& device)
  : PhysicalResource(device)
{
}
PhysicalBuffer::PhysicalBuffer(const wr::Device& device)
  : PhysicalResource(device)
{
}

PhysicalStage::~PhysicalStage()
{
  if (pipeline_ != VK_NULL_HANDLE)
    vkDestroyPipeline(device_.logical(), pipeline_, nullptr);
  if (layout_ != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(device_.logical(), layout_, nullptr);
}

PhysicalGraphicsStage::PhysicalGraphicsStage(const wr::Device& device)
  : PhysicalStage(device)
{
}

PhysicalGraphicsStage::~PhysicalGraphicsStage()
{
  if (render_pass_ != VK_NULL_HANDLE)
    vkDestroyRenderPass(device_.logical(), render_pass_, nullptr);
}

void BufferResource::addVertexAttribute(VkFormat format, uint32_t offset)
{
  vertex_attributes_.push_back({
    .location = static_cast<uint32_t>(vertex_attributes_.size()),
    .binding  = 0,
    .format   = format,
    .offset   = offset,
  });
}

void RenderStage::writesTo(const RenderResource* resource)
{
  writes_.push_back(resource);
}

void RenderStage::readsFrom(const RenderResource* resource)
{
  reads_.push_back(resource);
}

// TODO: I may have to rethink this whole thing. Could textures be a
// non-blocking read dep?
bool RenderStage::hasBlockingRead() const
{
  for (auto* resource : reads_) {
    if (const auto* buffer = resource->as<BufferResource>())
      switch (buffer->usage()) {
        case BufferUsage::index_buffer:
        case BufferUsage::vertex_buffer:
          break;
        default:
          return true;
      }
    else
      // Texture resources are a blocking read dep
      return true;
  }
  return false;
}

void GraphicsStage::bindBuffer(const BufferResource* buffer,
                               const uint32_t        binding)
{
  buffer_bindings_.emplace(buffer, binding);
}

void GraphicsStage::usesShader(const wr::Shader& shader)
{
  shaders_.push_back({
    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext               = {},
    .flags               = {},
    .stage               = shader.stage(),
    .module              = shader.module(),
    .pName               = shader.entryPoint().c_str(),
    .pSpecializationInfo = {},
  });
}

void RenderGraph::recordCommandBuffer(const RenderStage*       stage,
                                      const wr::CommandBuffer& cb,
                                      const uint32_t image_index) const
{
  const PhysicalStage& physical = *stage->physical_;

  // Record render pass for graphics stages.
  const auto* graphics_stage = stage->as<GraphicsStage>();
  if (graphics_stage != nullptr) {
    const auto* phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
    assert(phys_graphics_stage != nullptr);

    // TODO: fix
    // This array of clear value works, and is efficient, but restricts
    // the order in which TextureResource write dependencies can be added, as
    // the creation of attachments are done in the order of the stage->writes_
    // vector.
    //  - Since the color clear value is on index 0, and the
    //  back buffer/color buffer uses VK_ATTACHMENT_LOAD_OP_CLEAR, this
    //  buffer must be added as a write dependency first, giving it attachment
    //  index 0.
    //  - By the same token, the depth stencil buffer has to be added second
    // A better implementation is outlined in the comment in the RenderPass
    // constructor (renderpass.cpp)
    //
    // Note that fixing this problem will also most likely involve changing how
    // the framebuffers are created, as the image_views array order should
    // correspond to the attachment order.
    std::array<VkClearValue, 2> clear_values{};
    if (graphics_stage->clears_screen_) {
      // Black with 100% opacity
      clear_values[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
      clear_values[1].depthStencil = { 1.0f, 0 };
    }

    const VkRenderPassBeginInfo render_pass_bi{
      .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext       = nullptr,
      .renderPass  = phys_graphics_stage->render_pass_,
      .framebuffer = phys_graphics_stage->framebuffers_.at(image_index).get(),
      .renderArea{
        .offset = {},
        .extent = swapchain_.extent(),
      },
      .clearValueCount = static_cast<uint32_t>(clear_values.size()),
      .pClearValues    = clear_values.data(),
    };

    cb.beginRenderPass(render_pass_bi);
  }

  std::vector<VkBuffer> vertex_buffers;
  for (const auto* resource : stage->reads_) {
    const auto* buffer_resource = resource->as<BufferResource>();
    if (buffer_resource == nullptr) {
      continue;
    }

    auto physical_buffer = buffer_resource->physical_->as<PhysicalBuffer>();
    if (physical_buffer == nullptr) {
      continue;
    }
    if (physical_buffer->buffer_ == nullptr) {
      continue;
    }
    if (buffer_resource->usage_ == BufferUsage::index_buffer) {
      assert(physical_buffer->buffer_);
      cb.bindIndexBuffer(*physical_buffer->buffer_);
    }
    else if (buffer_resource->usage_ == BufferUsage::vertex_buffer) {
      vertex_buffers.push_back(physical_buffer->buffer_->get());
    }
  }

  if (!vertex_buffers.empty()) {
    cb.bindVertexBuffers(vertex_buffers);
  }

  cb.bindPipeline(physical.pipeline_);
  stage->on_record_(physical, cb);

  if (graphics_stage != nullptr) {
    cb.endRenderPass();
  }

  // TODO: Find a more performant solution instead of placing a full memory
  // barrier after each stage!
  cb.fullBarrier();
}

void RenderGraph::buildRenderPass(const GraphicsStage*   stage,
                                  PhysicalGraphicsStage& physical) const
{
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference>   resolve_refs;
  std::vector<VkAttachmentReference>   color_refs;
  std::vector<VkAttachmentReference>   depth_refs;
  // Build vulkan attachments. For every texture resource that stage writes to,
  // we create a corresponding VkAttachmentDescription and attach it to the
  // render pass.
  for (size_t i = 0; i < stage->writes_.size(); i++) {
    const auto* resource = stage->writes_[i];
    const auto* texture  = resource->as<TextureResource>();
    if (texture == nullptr) {
      continue;
    }

    VkAttachmentDescription attachment{};
    attachment.flags   = 0;
    attachment.format  = texture->format_;
    attachment.samples = stage->sample_count_;
    attachment.loadOp  = stage->clears_screen_ ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                               : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

    bool msaa_enabled{ stage->sample_count_ != VK_SAMPLE_COUNT_1_BIT };

    switch (texture->usage_) {
      case TextureUsage::back_buffer: {
        if (!stage->clears_screen_ && !msaa_enabled) {
          attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        attachment.samples = VK_SAMPLE_COUNT_1_BIT; // use only one sample when
                                                    // resolving to back buffer
        attachment.loadOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        const VkAttachmentReference bb_ref{
          static_cast<uint32_t>(i), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        if (msaa_enabled)
          resolve_refs.push_back(bb_ref);
        else
          color_refs.push_back(bb_ref);
      } break;
      case TextureUsage::color_buffer:
        assert(msaa_enabled);
        if (!stage->clears_screen_) {
          attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
          attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(
          { static_cast<uint32_t>(i), attachment.finalLayout });
        break;
      case TextureUsage::depth_stencil_buffer:
        attachment.finalLayout =
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        depth_refs.push_back(
          { static_cast<uint32_t>(i),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
        break;
      default:
        assert(false);
    }
    attachments.push_back(attachment);
  }

  const VkSubpassDescription subpass_description{
    .flags                = 0,
    .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments    = nullptr,
    .colorAttachmentCount = static_cast<std::uint32_t>(color_refs.size()),
    .pColorAttachments    = color_refs.data(),
    .pResolveAttachments  = resolve_refs.data(),
    .pDepthStencilAttachment =
      !depth_refs.empty() ? depth_refs.data() : nullptr,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments    = nullptr,
  };

  const VkSubpassDependency subpass_dependency{
    .srcSubpass   = VK_SUBPASS_EXTERNAL,
    .dstSubpass   = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    .dependencyFlags = 0,
  };

  const VkRenderPassCreateInfo render_pass_ci = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
    .pAttachments    = attachments.data(),
    .subpassCount    = 1,
    .pSubpasses      = &subpass_description,
    .dependencyCount = 1,
    .pDependencies   = &subpass_dependency,
  };
  if (const auto result = vkCreateRenderPass(device_.logical(), &render_pass_ci,
                                             nullptr, &physical.render_pass_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateRenderPass(): ");
  }
}

void RenderGraph::buildPipelineLayout(const RenderStage* stage,
                                      PhysicalStage&     physical) const
{
  const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
    .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext          = nullptr,
    .flags          = 0,
    .setLayoutCount = static_cast<uint32_t>(stage->descriptor_layouts_.size()),
    .pSetLayouts    = stage->descriptor_layouts_.data(),
    .pushConstantRangeCount =
      static_cast<uint32_t>(stage->push_constant_ranges_.size()),
    .pPushConstantRanges = stage->push_constant_ranges_.data(),
  };

  if (const auto result = vkCreatePipelineLayout(
        device_.logical(), &pipeline_layout_ci, nullptr, &physical.layout_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreatePipelineLayout(): failed for pipeline layout {}!",
            stage->name());
  }
}

void RenderGraph::buildGraphicsPipeline(const GraphicsStage*   stage,
                                        PhysicalGraphicsStage& physical) const
{
  // Build buffer and vertex layout bindings. For every buffer resource that
  // stage reads from, we create a corresponding attribute binding and vertex
  // binding description.
  std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
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
    const uint32_t binding = stage->buffer_bindings_.at(buffer_resource);
    for (auto attribute_description : buffer_resource->vertex_attributes_) {
      attribute_description.binding = binding;
      attribute_descriptions.push_back(attribute_description);
    }

    vertex_bindings.push_back({
      .binding   = binding,
      .stride    = static_cast<uint32_t>(buffer_resource->element_size_),
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
    .width    = static_cast<float>(swapchain_.extent().width),
    .height   = static_cast<float>(swapchain_.extent().height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  const VkRect2D scissor{
    .offset = { 0, 0 },
    .extent = swapchain_.extent(),
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
    .cullMode                = stage->cull_mode_,
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
    // TODO: rasterizationSamples should be passed as an argument based on the
    // textures in the stage in question. Also may want to add support for
    // single sample pipelines in which there are no resolve attachments
    .rasterizationSamples  = stage->sample_count_,
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
    .depthTestEnable       = stage->depth_test_ ? VK_TRUE : VK_FALSE,
    .depthWriteEnable      = stage->depth_write_ ? VK_TRUE : VK_FALSE,
    .depthCompareOp        = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable     = VK_FALSE,
    .front                 = {},
    .back                  = {},
    .minDepthBounds        = 0.0f, // optional
    .maxDepthBounds        = 1.0f, // optional
  };

  auto blend_attachment = stage->blend_attachment_;
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
    .pDynamicState       = &dynamic_state,
    .layout              = physical.layout_,
    .renderPass          = physical.render_pass_,
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
    .basePipelineIndex   = -1,
  };

  if (const auto result =
        vkCreateGraphicsPipelines(device_.logical(), nullptr, 1, &pipeline_ci,
                                  nullptr, &physical.pipeline_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateGraphicsPipelines(): failed for pipeline {}!",
            stage->name());
  }
}

void RenderGraph::compile()
{
  // TODO: the topology sorting implemented here has not been extensively
  // tested, but it works for the two stages I'm currently using...

  // Copy all stages to a new set that can be removed from when sorting
  std::vector<RenderStage*> stage_set{};
  for (auto& stage : stages_) {
    stage_set.push_back(stage.get());
  }

  std::vector<RenderStage*> q_set{}; // stages with no read dependencies
  // Initialize topology sort
  for (auto stage : stage_set) {
    if (!stage->hasBlockingRead())
      q_set.push_back(stage);
  }
  // Topology sort
  while (!q_set.empty()) {
    stage_stack_.emplace_back(q_set);
    for (auto q_stage : q_set) {
      for (auto write : q_stage->writes_) {
        for (auto g_stage : stage_set) {
          if (auto it = std::find(g_stage->reads_.begin(),
                                  g_stage->reads_.end(), write);
              it != g_stage->reads_.end()) {
            g_stage->reads_.erase(it);
          }
        }
      }
    }
    for (auto q_stage : q_set) {
      stage_set.erase(std::find(stage_set.begin(), stage_set.end(), q_stage));
    }
    q_set.clear();
    for (auto stage : stage_set) {
      if (!stage->hasBlockingRead())
        q_set.push_back(stage);
    }
  }
#ifdef _DEBUG
  std::ostringstream ss;
  ss << "{\n";
  for (size_t i = 0; i < stage_stack_.size(); ++i) {
    ss << "  Stage group " << i + 1 << ": {";
    for (size_t j = 0; j < stage_stack_[i].size() - 1; ++j) {
      ss << stage_stack_[i][j]->name() + ", ";
    }
    ss << stage_stack_[i].back()->name() << "}\n";
  }
  ss << "}";
  log_->debug("Proposed stage order:\n{}", ss.str());
#endif
  if (!stage_set.empty()) {
    ThrowVk(VkResult{}, "Render Graph contains cyclic dependencies!");
  }

  log_->trace("Allocating physical resource for buffers:");
  for (auto& buffer_resource : buffer_resources_) {
    log_->trace("   - {}", buffer_resource->name_);
    auto physical              = std::make_shared<PhysicalBuffer>(device_);
    buffer_resource->physical_ = physical;
  }

  log_->trace("Allocating physical resource for texture:");
  for (auto& texture_resource : texture_resources_) {
    log_->trace("   - {}", texture_resource->name());

    if (texture_resource->usage_ == TextureUsage::back_buffer) {
      texture_resource->physical_ =
        std::make_shared<PhysicalBackBuffer>(device_);
      continue;
    }

    const wr::ImageInfo texture_info = {
      .extent = swapchain_.extent(),
      .format = texture_resource->format_,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage_flags =
        texture_resource->usage_ == TextureUsage::depth_stencil_buffer
          ? static_cast<VkImageUsageFlags>(
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
          : static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
      .aspect_flags =
        texture_resource->usage_ == TextureUsage::depth_stencil_buffer
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT,
      .sample_count = texture_resource->sample_count_,
      .mip_levels   = 1,
    };

    const VmaAllocationCreateInfo alloc_ci{
      .flags          = {},
      .usage          = VMA_MEMORY_USAGE_GPU_ONLY,
      .requiredFlags  = {},
      .preferredFlags = {},
      .memoryTypeBits = {},
      .pool           = {},
      .pUserData      = {},
      .priority       = {},
    };

    auto physical    = std::make_shared<PhysicalImage>(device_);
    physical->image_ = std::make_unique<wr::GpuImage>(
      device_, texture_info, alloc_ci,
      fmt::format("{} image", texture_resource->name_));
    texture_resource->physical_ = physical;
  }

  for (auto substages : stage_stack_) {
    for (auto* stage : substages) {
      if (auto* graphics_stage = stage->as<GraphicsStage>()) {
        auto  physical_ptr = std::make_unique<PhysicalGraphicsStage>(device_);
        auto& physical     = *physical_ptr;
        graphics_stage->physical_ = std::move(physical_ptr);

        // Deduce sample count to be used in stage
        VkSampleCountFlagBits sample_count{ VK_SAMPLE_COUNT_1_BIT };
        for (auto* resource : stage->writes_) {
          if (const auto* texture = resource->as<TextureResource>()) {
            if (texture->sample_count_ > sample_count) {
              sample_count = texture->sample_count_;
            }
          }
        }
        graphics_stage->sample_count_ = sample_count;
        // log_->debug("Stage '{}' sample count: {}", graphics_stage->name_,
        //             static_cast<uint32_t>(sample_count));

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
          // The render graph renders to a single target
          // TODO: test if this is actually correct when using more images
          image_views.reserve(back_buffers.size() + images.size());
          for (auto* const img_view : swapchain_.imageViews()) {
            // the order of the image views need to correspond with the
            // attachment order in the VkRenderPass
            for (const auto* image : images) {
              image_views.push_back(image->image_->view());
            }
            std::fill_n(std::back_inserter(image_views), back_buffers.size(),
                        img_view);
            physical.framebuffers_.emplace_back(device_, physical.render_pass_,
                                                image_views, swapchain_);
            image_views.clear();
          }
        }
      }
    }
  }
}

void RenderGraph::render(uint32_t image_index, const wr::CommandBuffer& cb)
{
  for (auto& buffer_resource : buffer_resources_) {
    auto& physical = *buffer_resource->physical_->as<PhysicalBuffer>();
    switch (buffer_resource->on_render_) {
      case BufferResource::OnNextRender::create_new: {
        // Build buffer
        VkBufferUsageFlags buffer_usage{};
        switch (buffer_resource->usage_) {
          case BufferUsage::index_buffer:
            buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
          case BufferUsage::vertex_buffer:
            buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
          default:
            assert(false);
        }
        physical.buffer_ = std::make_unique<wr::GpuBuffer>(
          device_, buffer_resource->data_size_, buffer_usage,
          VMA_MEMORY_USAGE_CPU_TO_GPU, "render graph buffer");
        physical.buffer_->uploadData(buffer_resource->data_,
                                     buffer_resource->data_size_);
        break;
      }
      case BufferResource::OnNextRender::upload_only: {
        if (physical.buffer_ != nullptr) {
          physical.buffer_->uploadData(buffer_resource->data_,
                                       buffer_resource->data_size_);
        }
        break;
      }
      default:
        break;
    }
    buffer_resource->on_render_ = BufferResource::OnNextRender::skip;
  }

  // TODO: full memory barrier is not needed between nodes in same subset
  for (const auto& subset : stage_stack_) {
    for (const auto& stage : subset)
      recordCommandBuffer(stage, cb, image_index);
  }
}

} // namespace eldr::vk
