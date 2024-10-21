#include <eldr/core/logger.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk {

void RenderGraph::recordCommandBuffer(const RenderStage*       stage,
                                      const wr::CommandBuffer& cb,
                                      const std::uint32_t image_index) const
{
  const PhysicalStage& physical = *stage->physical_;

  // Record render pass for graphics stages.
  const auto* graphics_stage = stage->as<GraphicsStage>();
  if (graphics_stage != nullptr) {
    const auto* phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
    assert(phys_graphics_stage != nullptr);

    std::array<VkClearValue, 2> clear_values{};
    if (graphics_stage->clears_screen_) {
      clear_values[0].color        = { { 0, 0, 0, 0 } };
      clear_values[1].depthStencil = { 1.0f, 0 };
    }

    const VkRenderPassBeginInfo render_pass_bi{
      .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext       = nullptr,
      .renderPass  = phys_graphics_stage->render_pass_->get(),
      .framebuffer = phys_graphics_stage->framebuffers_.at(image_index).get(),
      .renderArea{
        .offset = {},
        .extent = swapchain_.extent(),
      },
      .clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
      .pClearValues    = clear_values.data(),
    };

    cb.beginRenderPass(render_pass_bi);
  }

  std::vector<wr::Buffer*> vertex_buffers;
  for (const auto* resource : stage->reads_) {
    const auto* buffer_resource = resource->as<BufferResource>();
    if (buffer_resource == nullptr) {
      continue;
    }

    wr::Buffer* physical_buffer = buffer_resource->physical_->as<wr::Buffer>();
    if (physical_buffer == nullptr) {
      continue;
    }
    if (buffer_resource->usage_ == BufferUsage::index_buffer) {
      cb.bindIndexBuffer(*physical_buffer);
    }
    else if (buffer_resource->usage_ == BufferUsage::vertex_buffer) {
      vertex_buffers.push_back(physical_buffer);
    }
  }

  if (!vertex_buffers.empty()) {
    cb.bindVertexBuffers(vertex_buffers);
  }

  cb.bindPipeline(*physical.pipeline_);
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
    attachment.samples = device_.maxMSAASampleCount();
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
    const std::uint32_t binding = stage->buffer_bindings_.at(buffer_resource);
    for (auto attribute_description : buffer_resource->vertex_attributes_) {
      attribute_description.binding = binding;
      attribute_descriptions.push_back(attribute_description);
    }

    vertex_bindings.push_back({
      .binding   = binding,
      .stride    = static_cast<std::uint32_t>(buffer_resource->element_size_),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    });
  }

  physical.pipeline_ =
    std::make_unique<wr::Pipeline>(device_, swapchain_, *stage, physical,
                                   attribute_descriptions, vertex_bindings);
}

void RenderGraph::compile(/*const RenderResource* target*/)
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
    ThrowVk({}, "Render Graph contains cyclic dependencies!");
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

  //  log_->trace("Allocating physical resource for buffers:");
  //
  //  for (auto& buffer_resource : buffer_resources_) {
  //    log_->trace("   - {}", buffer_resource->name_);
  //    auto physical              = std::make_shared<wr::Buffer>(device_);
  //    buffer_resource->physical_ = physical;
  //  }

  log_->trace("Allocating physical resource for texture:");
  for (auto& texture_resource : texture_resources_) {
    log_->trace(" - {}", texture_resource->name());

    if (texture_resource->usage_ == TextureUsage::back_buffer) {
      texture_resource->physical_ =
        std::make_shared<PhysicalBackBuffer>(device_, swapchain_);
      continue;
    }

    const wr::ImageInfo texture_info = {
      .extent      = swapchain_.extent(),
      .format      = texture_resource->format_,
      .tiling      = VK_IMAGE_TILING_OPTIMAL,
      .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT,
      .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
      .num_samples  = texture_resource->num_samples_,
      .mip_levels   = texture_resource->mip_levels_
    };

    VmaAllocationCreateInfo alloc_ci{};
    alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto physical =
      std::make_shared<wr::Image>(device_, texture_info, alloc_ci);
    texture_resource->physical_ = physical;

    // physical->image_ =
    //   std::make_unique<wr::Image>(device_, texture_info, alloc_ci);
    // physical->image_view_ = std::make_unique<wr::ImageView>(
    //   device_, *physical->image_, VK_IMAGE_ASPECT_COLOR_BIT);
  }

  for (auto substages : stage_stack_) {
    for (auto* stage : substages) {
      if (auto* graphics_stage = stage->as<GraphicsStage>()) {
        auto  physical_ptr = std::make_unique<PhysicalGraphicsStage>(device_);
        auto& physical     = *physical_ptr;
        graphics_stage->physical_ = std::move(physical_ptr);

        buildRenderPass(graphics_stage, physical);
        buildGraphicsPipeline(graphics_stage, physical);

        // If we write to at least one texture, we need to make framebuffers.
        if (!stage->writes_.empty()) {
          // For every texture that this stage writes to, we need to attach it
          // to the framebuffer.
          std::vector<const PhysicalBackBuffer*> back_buffers;
          std::vector<const wr::Image*>          images;
          for (const auto* resource : stage->writes_) {
            if (const auto* texture = resource->as<TextureResource>()) {
              const auto& physical_texture = *texture->physical_;
              if (const auto* back_buffer =
                    physical_texture.as<PhysicalBackBuffer>()) {
                back_buffers.push_back(back_buffer);
              }
              else if (const auto* image = physical_texture.as<wr::Image>()) {
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
              image_views.push_back(image->view());
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

void RenderGraph::render(uint32_t image_index, const wr::CommandBuffer& cb)
{
  for (auto& buffer_resource : buffer_resources_) {
    if (buffer_resource->data_upload_needed_) {

      // Build buffer
      VmaAllocationCreateInfo alloc_ci{};
      alloc_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
      alloc_ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
      auto physical =
        std::make_shared<wr::Buffer>(device_, *buffer_resource, alloc_ci);
      buffer_resource->physical_ = physical;

      physical->uploadData(buffer_resource->data_, buffer_resource->data_size_);
      buffer_resource->data_upload_needed_ = false;
    }
  }

  // TODO: full memory barrier is not needed between nodes in same sublist
  for (const auto& sublist : stage_stack_) {
    for (const auto& stage : sublist)
      recordCommandBuffer(stage, cb, image_index);
  }
}

} // namespace eldr::vk
