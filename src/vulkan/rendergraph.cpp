#include <eldr/core/platform.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

using namespace eldr::core;

namespace eldr::vk {
// void BufferResource::addVertexAttribute(VkFormat format, uint32_t offset)
// {
//   vertex_attributes_.push_back({
//     .location = static_cast<uint32_t>(vertex_attributes_.size()),
//     .binding  = 0,
//     .format   = format,
//     .offset   = offset,
//   });
// }

void RenderStage::writesTo(const RenderResource* resource)
{
  writes_.insert(resource);
}

void RenderStage::readsFrom(const RenderResource* resource)
{
  reads_.insert(resource);
}

bool RenderStage::hasReadDependency(
  const std::vector<std::unique_ptr<RenderStage>>& stages) const
{
  for (auto* resource : reads_) {
    for (auto& stage : stages) {
      if (stage.get() == this)
        continue;
      if (stage->writes_.contains(resource)) {
        return true;
      }
    }
  }
  return false;
}

// void GraphicsStage::bindBuffer(const BufferResource* buffer,
//                                const uint32_t        binding)
// {
//   buffer_bindings_.emplace(buffer, binding);
// }

// void GraphicsStage::usesShader(const wr::Shader& shader)
// {
//   shaders_.push_back({
//     .sType               =
//     VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .pNext = {}, .flags
//     = {}, .stage               = shader.stage(), .module              =
//     shader.module(), .pName               = shader.entryPoint().c_str(),
//     .pSpecializationInfo = {},
//   });
// }

void RenderGraph::recordCommandBuffer(const RenderStage*       stage,
                                      const wr::CommandBuffer& cb,
                                      uint32_t frame_index) const
{
  const PhysicalStage& physical = *stage->physical_;

  // Record render pass for graphics stages.
  const auto* graphics_stage = stage->as<GraphicsStage>();
  if (graphics_stage != nullptr) {
    const auto* phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
    assert(phys_graphics_stage != nullptr);

    const VkRenderingInfo render_info{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .renderArea{
        .offset{},
        .extent{ swapchain_.extent() },
      },
      .layerCount           = 1,
      .viewMask             = 0,
      .colorAttachmentCount = static_cast<uint32_t>(
        phys_graphics_stage->color_attachments_[frame_index].size()),
      .pColorAttachments =
        phys_graphics_stage->color_attachments_[frame_index].data(),
      .pDepthAttachment   = phys_graphics_stage->depth_attachment_.get(),
      .pStencilAttachment = VK_NULL_HANDLE,
    };
    cb.beginRendering(render_info);
  }

  std::vector<VkBuffer> vertex_buffers;
  for (const auto* resource : stage->reads_) {
    auto* buffer_resource{ resource->as<BufferResource>() };
    if (buffer_resource == nullptr)
      continue;

    if (const auto& physical_buffer{
          dynamic_pointer_cast<PhysicalBuffer>(buffer_resource->physical_) }) {

      if (unlikely(physical_buffer->buffer_.empty())) {
        Log(Debug,
            "The PhysicalBuffer of '{}' in stage '{}' is empty. "
            "This can happen when RenderGraph::render(...) gets called "
            "before data has been uploaded to the buffer via "
            "BufferResource::uploadData(...), or it could be "
            "caused by some other bug.",
            buffer_resource->name_,
            stage->name_);
        return;
      }

      if (buffer_resource->buffer_usage_ & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
        assert(physical_buffer->buffer_.get());
        cb.bindIndexBuffer(physical_buffer->buffer_);
      }

      else if (buffer_resource->buffer_usage_ &
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
        vertex_buffers.push_back(physical_buffer->buffer_.get());
      }
    }
  }

  // if (!vertex_buffers.empty()) {
  //   cb.bindVertexBuffers(vertex_buffers);
  // }
  // cb.bindPipeline(physical.pipeline_);
  stage->on_record_(physical, cb);

  if (graphics_stage != nullptr) {
    cb.endRendering();
  }

  // TODO: Find a more performant solution instead of placing a full memory
  // barrier after each stage!
  cb.fullBarrier();
}

void RenderGraph::buildAttachments(const GraphicsStage*   stage,
                                   PhysicalGraphicsStage& physical) const
{
  for (uint32_t i{ 0 }; i < max_frames_in_flight; ++i) {
    auto& attachments = physical.color_attachments_[i];
    attachments.clear();
    for (const auto* resource : stage->writes_) {
      const auto* texture = resource->as<TextureResource>();
      if (texture == nullptr) {
        continue;
      }
      if (const auto* image = texture->physical_->as<PhysicalImage>()) {
        VkRenderingAttachmentInfo attachment{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .pNext              = {},
          .imageView          = image->image_.view().get(),
          .imageLayout        = image->image_.layout(),
          .resolveMode        = texture->sample_count_ == VK_SAMPLE_COUNT_1_BIT
                                  ? VK_RESOLVE_MODE_NONE
                                  : VK_RESOLVE_MODE_AVERAGE_BIT,
          .resolveImageView   = {},
          .resolveImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .loadOp     = stage->clears_screen_ ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                              : VK_ATTACHMENT_LOAD_OP_LOAD,
          .storeOp    = VK_ATTACHMENT_STORE_OP_STORE,
          .clearValue = {}
        };

        attachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        switch (texture->usage_) {
          case TextureUsage::ColorBuffer:
            attachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
            // TODO: Other resolve targets than swapchain image should be
            // possible
            attachment.resolveImageView = swapchain_.imageView(i).get();
            attachments.push_back(attachment);
            break;
          case TextureUsage::DepthStencilBuffer:
            attachment.clearValue.depthStencil = { 1.0f, 0 };
            physical.depth_attachment_ =
              std::make_unique<VkRenderingAttachmentInfo>(attachment);
            break;
          default:
            Assert(false);
        }
      }
    }
  }
}

// void RenderGraph::buildRenderPass(const GraphicsStage*   stage,
//                                   PhysicalGraphicsStage& physical) const
// {
//   std::vector<VkAttachmentDescription> attachments;
//   std::vector<VkAttachmentReference>   resolve_refs;
//   std::vector<VkAttachmentReference>   color_refs;
//   std::vector<VkAttachmentReference>   depth_refs;
//   // Build vulkan attachments. For every texture resource that stage writes
//   to,
//   // we create a corresponding VkAttachmentDescription and attach it to the
//   // render pass.
//   for (size_t i = 0; i < stage->writes_.size(); i++) {
//     const auto* resource = stage->writes_[i];
//     const auto* texture  = resource->as<TextureResource>();
//     if (texture == nullptr) {
//       continue;
//     }
//
//     VkAttachmentDescription attachment{};
//     attachment.flags   = 0;
//     attachment.format  = texture->format_;
//     attachment.samples = stage->sample_count_;
//     attachment.loadOp  = stage->clears_screen_ ?
//     VK_ATTACHMENT_LOAD_OP_CLEAR
//                                                :
//                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
//
//     bool msaa_enabled{ stage->sample_count_ != VK_SAMPLE_COUNT_1_BIT };
//
//     switch (texture->usage_) {
//       case TextureUsage::BackBuffer: {
//         if (!stage->clears_screen_ && !msaa_enabled) {
//           attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//           attachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
//         }
//         attachment.samples = VK_SAMPLE_COUNT_1_BIT; // use only one sample
//         when
//                                                     // resolving to back
//                                                     buffer
//         attachment.loadOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//         attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//         const VkAttachmentReference bb_ref{
//           static_cast<uint32_t>(i),
//           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//         };
//         if (msaa_enabled)
//           resolve_refs.push_back(bb_ref);
//         else
//           color_refs.push_back(bb_ref);
//       } break;
//       case TextureUsage::ColorBuffer:
//         assert(msaa_enabled);
//         if (!stage->clears_screen_) {
//           attachment.initialLayout =
//           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; attachment.loadOp =
//           VK_ATTACHMENT_LOAD_OP_LOAD;
//         }
//         attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//         color_refs.push_back(
//           { static_cast<uint32_t>(i), attachment.finalLayout });
//         break;
//       case TextureUsage::DepthStencilBuffer:
//         attachment.finalLayout =
//           VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//         depth_refs.push_back(
//           { static_cast<uint32_t>(i),
//             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
//         break;
//       default:
//         assert(false);
//     }
//     attachments.push_back(attachment);
//   }
//
//   const VkSubpassDescription subpass_description{
//     .flags                = 0,
//     .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
//     .inputAttachmentCount = 0,
//     .pInputAttachments    = nullptr,
//     .colorAttachmentCount = static_cast<std::uint32_t>(color_refs.size()),
//     .pColorAttachments    = color_refs.data(),
//     .pResolveAttachments  = resolve_refs.data(),
//     .pDepthStencilAttachment =
//       !depth_refs.empty() ? depth_refs.data() : nullptr,
//     .preserveAttachmentCount = 0,
//     .pPreserveAttachments    = nullptr,
//   };
//
//   const VkSubpassDependency subpass_dependency{
//     .srcSubpass   = VK_SUBPASS_EXTERNAL,
//     .dstSubpass   = 0,
//     .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
//                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//     .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
//                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//     .srcAccessMask = 0,
//     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
//                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
//     .dependencyFlags = 0,
//   };
//
//   // TODO: make a render pass builder? see how it pans out with dynamic
//   // rendering, dunno where I will end up
//   physical.render_pass_ =
//     wr::RenderPass{ device_, attachments, subpass_description,
//                     subpass_dependency };
// }

// void RenderGraph::buildPipelineLayout(const RenderStage* stage,
//                                       PhysicalStage&     physical) const
//{
//   const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
//     .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//     .pNext          = nullptr,
//     .flags          = 0,
//     .setLayoutCount =
//     static_cast<uint32_t>(stage->descriptor_layouts_.size()), .pSetLayouts
//     = stage->descriptor_layouts_.data(), .pushConstantRangeCount =
//       static_cast<uint32_t>(stage->push_constant_ranges_.size()),
//     .pPushConstantRanges = stage->push_constant_ranges_.data(),
//   };
//
//   if (const auto result = vkCreatePipelineLayout(
//         device_.logical(), &pipeline_layout_ci, nullptr,
//         &physical.pipelineLayout());
//       result != VK_SUCCESS) {
//     ThrowVk(result, "vkCreatePipelineLayout(): failed for pipeline layout
//     {}!",
//             stage->name());
//   }
// }

// void RenderGraph::buildGraphicsPipeline(const GraphicsStage*   stage,
//                                         PhysicalGraphicsStage& physical)
//                                         const
// {
//
//   // Build buffer and vertex layout bindings. For every buffer resource
//   that
//   // stage reads from, we create a corresponding attribute binding and
//   vertex
//   // binding description.
//   // std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
//   // std::vector<VkVertexInputBindingDescription>   vertex_bindings;
//   // for (const auto* resource : stage->reads_) {
//   //   const auto* buffer_resource = resource->as<BufferResource>();
//   //   if (buffer_resource == nullptr) {
//   //     continue;
//   //   }
//
//   //   // Don't mess with index buffers here.
//   //   if (buffer_resource->usage_ == BufferUsage::IndexBuffer) {
//   //     continue;
//   //   }
//
//   //   // We use std::unordered_map::at() here to ensure that a binding
//   value
//   //   // exists for buffer_resource.
//   //   const uint32_t binding =
//   stage->buffer_bindings_.at(buffer_resource);
//   //   for (auto attribute_description :
//   buffer_resource->vertex_attributes_)
//   {
//   //     attribute_description.binding = binding;
//   //     attribute_descriptions.push_back(attribute_description);
//   //   }
//
//   //   vertex_bindings.push_back({
//   //     .binding   = binding,
//   //     .stride    =
//   static_cast<uint32_t>(buffer_resource->element_size_),
//   //     .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
//   //   });
//   // }
//
//   PipelineBuilder pipeline_builder;
//   pipeline_builder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
//     .setPolygonMode(VK_POLYGON_MODE_FILL)
//     .setCullMode(stage->cull_mode_,
//                  VK_FRONT_FACE_COUNTER_CLOCKWISE) // cull mode should
//                  probably
//                                                   // not be a member of
//                                                   stage
//     .setMultisampling(stage->sample_count_)
//     .setDepthFormat(device_.findDepthFormat()) // should be set from depth
//                                                // texture in the end
//     .enableBlendingAlphaBlend();
//   physical.pipeline_ = pipeline_builder.build(device_, stage->name());
// }

void RenderGraph::compile()
{
  // TODO: the topology sorting implemented here has not been extensively
  // tested, but it works for the two stages I'm currently using...

  // We make a copy of all reads to use when topology sorting to avoid
  // modifying the real graph
  std::unordered_map<RenderStage*, std::unordered_set<const RenderResource*>>
    reads;
  for (auto& stage : stages_) {
    reads[stage.get()] = stage->reads_;
  }

  // Set up the topology sort
  std::vector<RenderStage*> q_set{}; // stages with no read dependencies
  for (auto& kv : reads) {
    bool has_read_dep = false;
    for (const auto* resource : kv.second) {
      for (const auto& stage : stages_) {
        if (stage.get() == kv.first)
          continue;
        if (stage->writes_.contains(resource)) {
          has_read_dep = true;
        }
      }
    }
    if (!has_read_dep) {
      reads[kv.first].clear();
      q_set.push_back(kv.first);
    }
  }

  // Topology sort
  // This doesn't exactly follow the standard topology sort algorithm 1-1 since
  // I'm not interested in having a flat vector of sorted nodes, but rather a
  // vector of vectors where each subvector of nodes can be recorded onto the
  // same command buffer without a memory barrier between. Another way of doing
  // this would be to first do a regular topology sort where the nodes end up in
  // a flat vector, and then figure out where the memory barriers are needed.
  while (!q_set.empty()) {
    stage_stack_.emplace_back(q_set);
    q_set.clear();
    for (auto q_stage : stage_stack_.back()) {
      for (auto write : q_stage->writes_) {
        for (auto& kv : reads) {
          if (reads[kv.first].contains(write)) {
            reads[kv.first].erase(write);
            if (reads[kv.first].empty())
              q_set.push_back(kv.first);
          }
        }
      }
    }
  }
  // Does any node still have incoming edges? If so, the graph is invalid
  for (auto& kv : reads) {
    if (!kv.second.empty()) {
      Throw("Render Graph contains cyclic dependencies!");
    }
  }
#ifdef DEBUG
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
  Log(Debug, "Proposed stage order:\n{}", ss.str());
#endif

  Log(Trace, "Allocating physical resource for buffers:");
  for (auto& buffer_resource : buffer_resources_) {
    Log(Trace, "   - {}", buffer_resource->name_);
    buffer_resource->physical_ = std::make_shared<PhysicalBuffer>();
  }

  Log(Trace, "Allocating physical resource for texture:");
  for (auto& texture : texture_resources_) {
    Log(Trace, "   - {}", texture->name());

    // if (texture_resource->usage_ == TextureUsage::BackBuffer) {
    //   texture_resource->physical_ = std::make_shared<PhysicalBackBuffer>();
    //   continue;
    // }
    const bool color_usage{ texture->usage_ == TextureUsage::ColorBuffer };
    const wr::Image::ImageCreateInfo texture_info{
      .name        = fmt::format("{} image", texture->name_),
      .extent      = swapchain_.extent(),
      .format      = texture->format_,
      .tiling      = VK_IMAGE_TILING_OPTIMAL,
      .usage_flags = color_usage ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                 : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .aspect_flags =
        color_usage ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT,
      .sample_count = texture->sample_count_,
      .mip_levels   = 1,
      .memory_usage = VMA_MEMORY_USAGE_GPU_ONLY,
      .final_layout = color_usage ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                  : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    };

    auto physical      = std::make_shared<PhysicalImage>();
    physical->image_   = wr::Image{ device_, texture_info };
    texture->physical_ = physical;
  }

  for (auto substages : stage_stack_) {
    for (auto* stage : substages) {
      if (auto* graphics_stage = stage->as<GraphicsStage>()) {
        auto g_stage_ptr = std::make_unique<PhysicalGraphicsStage>();
        // Need PhysicalGraphicsStage& later
        auto& physical            = *g_stage_ptr;
        graphics_stage->physical_ = std::move(g_stage_ptr); // upcasts

        // Deduce sample count to be used in stage
        // VkSampleCountFlagBits sample_count{ VK_SAMPLE_COUNT_1_BIT };
        // for (auto* resource : stage->writes_) {
        //   if (const auto* texture = resource->as<TextureResource>()) {
        //     if (texture->sample_count_ > sample_count) {
        //       sample_count = texture->sample_count_;
        //     }
        //   }
        // }
        // graphics_stage->sample_count_ = sample_count;
        //  log_->debug("Stage '{}' sample count: {}", graphics_stage->name_,
        //              static_cast<uint32_t>(sample_count));

        buildAttachments(graphics_stage, physical);
        // buildRenderPass(graphics_stage, physical);
        // buildPipelineLayout(graphics_stage, physical);
        // buildGraphicsPipeline(graphics_stage, physical);
      }
    }
  }
}

void RenderGraph::render(const wr::CommandBuffer& cb, uint32_t frame_index)
{
  for (auto& buffer_resource : buffer_resources_) {
    if (buffer_resource->data_.data() != nullptr) {
      // There is data to be uploaded to the gpu
      const size_t data_size{ buffer_resource->data_.size_bytes() };
      auto&        physical{ *dynamic_pointer_cast<PhysicalBuffer>(
        buffer_resource->physical_) };
      if (data_size == 0) {
        // Free the buffer
        // TODO: Decide whether BufferResource::uploadData() should allow
        // upploading empty spans at all. Doing so is most likely a
        // mistake (as far as I can see right now).
        // physical.buffer_.reset();
        physical.buffer_ = {};
      }
      else {
        bool new_buffer_needed = false;
        if (unlikely(physical.buffer_.empty())) {
          new_buffer_needed = true;
        }
        else {
          // A gpu buffer already exists
          if (data_size != physical.buffer_.size_bytes()) {
            // The gpu buffer needs to be resized
            new_buffer_needed = true;
          }
        }

        if (new_buffer_needed) {
          // Otherwise build a new GPU buffer
          physical.buffer_ =
            wr::GpuBuffer<byte_t>{ device_,
                                   buffer_resource->name(),
                                   data_size,
                                   buffer_resource->buffer_usage_,
                                   buffer_resource->memory_usage_ };
        }
      }
      // Upload data
      physical.buffer_.uploadData(buffer_resource->data_);
      // Reset data pointer once it has been uploaded to gpu
      buffer_resource->data_ = {};
    }
  }

  // TODO: full memory barrier is not needed between nodes in same subset
  for (const auto& subset : stage_stack_) {
    for (const auto& stage : subset)
      recordCommandBuffer(stage, cb, frame_index);
  }
}

} // namespace eldr::vk
