#include <eldr/core/logger.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
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
PhysicalGraphicsStage::PhysicalGraphicsStage(const wr::Device& device)
  : PhysicalStage(device)
{
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
                               const std::uint32_t   binding)
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
      clear_values[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
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
    if (buffer_resource->usage_ == BufferUsage::index_buffer) {
      cb.bindIndexBuffer(*physical_buffer->buffer_);
    }
    else if (buffer_resource->usage_ == BufferUsage::vertex_buffer) {
      vertex_buffers.push_back(physical_buffer->buffer_->get());
    }
  }

  if (!vertex_buffers.empty()) {
    cb.bindVertexBuffers(vertex_buffers);
  }

  // TODO: viewport and scissor are defined as dynamic states in pipeline so
  // they must be set. Here they are set to the same thing every time. May want
  // to set them elsewhere if the need arises.
  const VkViewport viewports[] = { {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = static_cast<float>(swapchain_.extent().width),
    .height   = static_cast<float>(swapchain_.extent().height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  } };
  cb.setViewport(viewports, 0);

  const VkRect2D scissors[] = { {
    .offset = { 0, 0 },
    .extent = swapchain_.extent(),
  } };
  cb.setScissor(scissors, 0);

  cb.bindPipeline(physical.pipeline_->get());
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
  std::vector<VkAttachmentReference>   offscreen_refs;
  std::vector<VkAttachmentReference>   depth_refs;

  // Build vulkan attachments. For every texture resource that stage writes to,
  // we create a corresponding VkAttachmentDescription and attach it to the
  // render pass.
  for (std::size_t i = 0; i < stage->writes_.size(); i++) {
    const auto* resource = stage->writes_[i];
    const auto* texture  = resource->as<TextureResource>();
    if (texture == nullptr) {
      continue;
    }

    VkAttachmentDescription attachment{};
    attachment.flags   = 0;
    attachment.format  = texture->format_;
    attachment.samples = texture->msaa_sample_count_;
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
        resolve_refs.push_back({ static_cast<std::uint32_t>(i),
                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        break;
      case TextureUsage::offscreen_buffer:
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        offscreen_refs.push_back(
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
        assert(false);
    }
    attachments.push_back(attachment);
  }
  physical.render_pass_ = std::make_unique<wr::RenderPass>(
    device_, attachments, resolve_refs, offscreen_refs, depth_refs);
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
    ss << stage_stack_[i].back()->name() << "}";
  }
  ss << "\n}";
  log_->debug("Proposed stage order:\n{}", ss.str());
#endif
  if (!stage_set.empty()) {
    ThrowVk({}, "Render Graph contains cyclic dependencies!");
  }

  // std::unordered_map<const RenderResource*, std::vector<RenderStage*>>
  // writers; for (auto& stage : stages_) {
  //   for (const auto* resource : stage->writes_) {
  //     writers[resource].push_back(stage.get());
  //   }
  // }

  log_->trace("Allocating physical resource for buffers:");

  for (auto& buffer_resource : buffer_resources_) {
    log_->trace("   - {}", buffer_resource->name_);
    auto physical              = std::make_shared<PhysicalBuffer>(device_);
    buffer_resource->physical_ = physical;
  }

  log_->trace("Allocating physical resource for texture:");

  for (auto& texture_resource : texture_resources_) {
    log_->trace(" - {}", texture_resource->name());

    if (texture_resource->usage_ == TextureUsage::back_buffer) {
      texture_resource->physical_ =
        std::make_shared<PhysicalBackBuffer>(device_, swapchain_);
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
      .sample_count = texture_resource->msaa_sample_count_,
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
              image_views.push_back(image->image_->view());
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
      auto& physical = *buffer_resource->physical_->as<PhysicalBuffer>();

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
      buffer_resource->data_upload_needed_ = false;
    }
  }

  // TODO: full memory barrier is not needed between nodes in same subset
  for (const auto& subset : stage_stack_) {
    for (const auto& stage : subset)
      recordCommandBuffer(stage, cb, image_index);
  }
}

} // namespace eldr::vk
