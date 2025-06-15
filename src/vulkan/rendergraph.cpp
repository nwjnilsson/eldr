#include <eldr/core/platform.hpp>
#include <eldr/vulkan/engine.hpp>
#include <eldr/vulkan/pipelinebuilder.hpp>
#include <eldr/vulkan/rendergraph.hpp>
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>

#include <deque>

NAMESPACE_BEGIN(eldr::vk)
RenderStage& RenderStage::writesTo(const RenderResource* resource)
{
  if (unlikely(writes_.contains(resource))) {
    Log(Warn,
        "{} has already been added to the writes of \"{}\"",
        resource->name(),
        this->name());
  }
  writes_.insert(resource);
  return *this;
}

RenderStage& RenderStage::readsFrom(const RenderResource* resource)
{
  reads_.insert(resource);
  return *this;
}

GraphicsStage& GraphicsStage::writesTo(const TextureResource* resource,
                                       VkAttachmentLoadOp     load_op,
                                       VkAttachmentStoreOp    store_op)
{
  RenderStage::writesTo(resource);
  load_store_ops_.insert(
    std::make_pair(resource, std::make_pair(load_op, store_op)));
  return *this;
}

GraphicsStage& GraphicsStage::readsFrom(const RenderResource* resource)
{
  RenderStage::readsFrom(resource);
  return *this;
}

// void TextureResource::setFlags(TextureFlags flags)
// {
// #ifdef DEBUG
//   if (flags & TextureFlags::Presentable) {
//     Assert(sample_count_ == VK_SAMPLE_COUNT_1_BIT,
//            "The sample count of presentable textures must be 1.");
//   }
// #endif
//   flags_ = flags;
// }

void TextureResource::setSampleCount(VkSampleCountFlagBits sample_count)
{
  // #ifdef DEBUG
  //   if (flags_ & TextureFlags::Presentable) {
  //     Assert(sample_count == VK_SAMPLE_COUNT_1_BIT,
  //            "The sample count of presentable textures must be 1.");
  //   }
  // #endif
  sample_count_ = sample_count;
}

void TextureResource::resolvesTo(TextureResource* target)
{
  Assert(target->sample_count_ == VK_SAMPLE_COUNT_1_BIT);
  resolve_ = target;
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

void RenderGraph::recordCommandBuffer(const RenderStage*       stage,
                                      const wr::CommandBuffer& cb) const
{
  const PhysicalStage& physical = *stage->physical_;

  // Record render pass for graphics stages.
  const auto* graphics_stage = stage->as<GraphicsStage>();
  if (graphics_stage != nullptr) {
    const auto* phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
    Assert(phys_graphics_stage != nullptr);

    const VkRenderingInfo render_info{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .renderArea{
        .offset{},
        .extent{ swapchain_.extent() },
      },
      .layerCount = 1,
      .viewMask   = 0,
      .colorAttachmentCount =
        static_cast<uint32_t>(phys_graphics_stage->color_attachments_.size()),
      .pColorAttachments  = phys_graphics_stage->color_attachments_.data(),
      .pDepthAttachment   = phys_graphics_stage->depth_attachment_.get(),
      .pStencilAttachment = VK_NULL_HANDLE,
    };
    cb.beginRendering(render_info);
  }

  // std::vector<VkBuffer> vertex_buffers;
  for (const auto* resource : stage->reads_) {
    auto* buffer_resource{ resource->as<BufferResource>() };
    if (buffer_resource == nullptr)
      continue;

    if (const auto& physical_buffer =
          buffer_resource->physical_->as<PhysicalBuffer>()) {

      if (unlikely(physical_buffer->buffer_.empty())) {
        Log(Debug,
            "The PhysicalBuffer of '{}' in stage '{}' is empty. "
            "This can happen when RenderGraph::render(...) gets called "
            "before data has been uploaded to the buffer via "
            "BufferResource::uploadData(...).",
            buffer_resource->name_,
            stage->name_);
        return;
      }

      // if (buffer_resource->buffer_usage_ & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
      // {
      //   Assert(physical_buffer->buffer_.vk());
      //   cb.bindIndexBuffer(physical_buffer->buffer_);
      // }

      // else if (buffer_resource->buffer_usage_ &
      //          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
      //   vertex_buffers.push_back(physical_buffer->buffer_.vk());
      // }
    }
  }

  // if (not vertex_buffers.empty()) {
  //   cb.bindVertexBuffers(vertex_buffers);
  // }
  // cb.bindPipeline(physical.pipeline_);
  stage->on_record_(cb);

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
  auto& attachments = physical.color_attachments_;
  attachments.clear();
  for (const auto* resource : stage->writes_) {
    const auto* texture = resource->as<TextureResource>();
    if (texture == nullptr) {
      continue;
    }
    if (stage->resolves_.contains(texture)) {
      continue;
    }
    if (const auto* image = texture->physical_->as<PhysicalImage>()) {
      const PhysicalImage*  resolve_image;
      VkImageView           resolve_view{ VK_NULL_HANDLE };
      VkImageLayout         resolve_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
      VkResolveModeFlagBits resolve_flags{ VK_RESOLVE_MODE_NONE };
      if (texture->resolve_ != nullptr) {
        resolve_image = texture->resolve_->physical_->as<PhysicalImage>();
        Assert(resolve_image, "Invalid resolve image. Has it been created?");
        Assert(texture->sample_count_ != VK_SAMPLE_COUNT_1_BIT,
               "A resolve image has been provided, but only a single sample "
               "is used.");
        resolve_view   = resolve_image->image_.view().vk();
        resolve_layout = resolve_image->image_.layout();
        resolve_flags  = VK_RESOLVE_MODE_AVERAGE_BIT;
      }

      const VkAttachmentLoadOp load_op{
        stage->load_store_ops_.at(texture).first
      };
      const VkAttachmentStoreOp store_op{
        stage->load_store_ops_.at(texture).second
      };
      VkRenderingAttachmentInfo attachment{
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext              = {},
        .imageView          = image->image_.view().vk(),
        .imageLayout        = image->image_.layout(),
        .resolveMode        = resolve_flags,
        .resolveImageView   = resolve_view,
        .resolveImageLayout = resolve_layout,
        .loadOp             = load_op,
        .storeOp            = store_op,
        .clearValue         = texture->clear_value_,
      };

      switch (texture->usage_) {
        case TextureUsage::Color:
          attachments.push_back(attachment);
          break;
        case TextureUsage::DepthStencil:
          physical.depth_attachment_ =
            std::make_unique<VkRenderingAttachmentInfo>(attachment);
          break;
        default:
          Throw("Rendering attachment for this texture usage has not been "
                "implemented.");
      }
    }
  }
}

void RenderGraph::compile()
{
  Log(Trace, "Compiling render graph...");
#ifdef DEBUG
  // ---------------------------------------------------------------------------
  // Validity checks
  // There is some nasty code in this section but it is mainly a placeholder for
  // more advanced validation/scheduling algorithms that might come later.
  // ---------------------------------------------------------------------------
  // Check that each texture resource is only cleared by one stage
  for (const auto& texture : texture_resources_) {
    int count{ 0 };
    for (const auto& stage : stages_) {
      if (const auto* g_stage = stage->as<GraphicsStage>()) {
        if (g_stage->writes_.contains(texture.get())) {
          if (g_stage->load_store_ops_.at(texture.get()).first ==
              VK_ATTACHMENT_LOAD_OP_CLEAR) {
            count++;
          }
        }
      }
    }
    if (count > 1) {
      Throw("Texture resource \"{}\" is cleared by more than one stage.",
            texture->name_);
    }
  }

  // Check for unschedulable WAW dependency where:
  // Stage A writes to resource X and Y with LoadOp::Clear and LoadOp::Load,
  // respectively.
  // Stage B writes to resource X and Y with LoadOp::Load and
  // LoadOp::Clear, respectively.
  // In this scenario, neither A nor B can be scheduled first because either X
  // or Y will be cleared after being written to. There's probably a much
  // smarter and more efficient way to do this but this seems to work for now.
  for (size_t i{ 0 }; i < stages_.size(); ++i) {
    const auto* g_stage1 = stages_[i]->as<GraphicsStage>();
    if (not g_stage1) {
      continue;
    }
    for (size_t j{ i + 1 }; j < stages_.size(); ++j) {
      const auto* g_stage2 = stages_[j]->as<GraphicsStage>();
      if (not g_stage2) {
        continue;
      }
      std::unordered_set<const TextureResource*> candidates;
      for (const auto* write1 : g_stage1->writes_) {
        const auto* texture1 = write1->as<TextureResource>();
        if (not texture1) {
          continue;
        }

        if ((g_stage2->writes_.contains(texture1)) or
            (texture1->resolve_ and
             g_stage2->writes_.contains(texture1->resolve_))) {
          candidates.insert(texture1);
        }
      }
      for (auto it1 = candidates.begin(); it1 != candidates.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != candidates.end(); ++it2) {
          const auto* const      t1 = *it1;
          const auto* const      t2 = *it2;
          const TextureResource* k11;
          const TextureResource* k12;
          const TextureResource* k21;
          const TextureResource* k22;
          if (g_stage1->writes_.contains(t1)) {
            k11 = t1;
          }
          else {
            k11 = t1->resolve_;
            Assert(t1->resolve_);
          }
          if (g_stage1->writes_.contains(t2)) {
            k12 = t2;
          }
          else {
            k12 = t2->resolve_;
            Assert(t2->resolve_);
          }
          if (g_stage2->writes_.contains(t1)) {
            k21 = t1;
          }
          else {
            k21 = t1->resolve_;
            Assert(t1->resolve_);
          }
          if (g_stage2->writes_.contains(t2)) {
            k22 = t2;
          }
          else {
            k22 = t2->resolve_;
            Assert(t2->resolve_);
          }
          const VkAttachmentLoadOp l11{
            g_stage1->load_store_ops_.at(k11).first
          };
          const VkAttachmentLoadOp l12{
            g_stage1->load_store_ops_.at(k12).first
          };
          const VkAttachmentLoadOp l21{
            g_stage2->load_store_ops_.at(k21).first
          };
          const VkAttachmentLoadOp l22{
            g_stage2->load_store_ops_.at(k22).first
          };
          if ((l11 == VK_ATTACHMENT_LOAD_OP_CLEAR and
               l12 == VK_ATTACHMENT_LOAD_OP_LOAD and
               l21 == VK_ATTACHMENT_LOAD_OP_LOAD and
               l22 == VK_ATTACHMENT_LOAD_OP_CLEAR) or
              (l11 == VK_ATTACHMENT_LOAD_OP_LOAD and
               l12 == VK_ATTACHMENT_LOAD_OP_CLEAR and
               l21 == VK_ATTACHMENT_LOAD_OP_CLEAR and
               l22 == VK_ATTACHMENT_LOAD_OP_LOAD)) {
            Throw("Unschedulable WAW dependency detected between stages \"{}\" "
                  "and \"{}\"",
                  g_stage1->name_,
                  g_stage2->name_);
          }
        }
      }
    }
  }

  // Check MSAA sample counts for textures and resolve targets
  for (const auto& texture : texture_resources_) {
    if (texture->resolve_) {
      if (texture->sample_count_ == VK_SAMPLE_COUNT_1_BIT)
        Throw("Texture {} has an invalid sample count ({}). A resolve "
              "attachment has been defined (\"{}\"), thus the sample count "
              "should be greater than 1.",
              texture->name_,
              texture->sample_count_,
              texture->resolve_->name_);
      if (texture->resolve_->sample_count_ != VK_SAMPLE_COUNT_1_BIT) {
        Throw("The resolve attachment \"{}\" of texture \"{}\" has an invalid "
              "sample count: {}, (should be VK_SAMPLE_COUNT_1_BIT).",
              texture->resolve_->name_,
              texture->resolve_->name_,
              texture->resolve_->sample_count_);
      }
    }
  }
#endif

  // ---------------------------------------------------------------------------
  // Add resolve textures to write sets
  // ---------------------------------------------------------------------------
  for (auto& stage : stages_) {
    std::vector<TextureResource*> resolves;
    if (auto* g_stage = stage->as<GraphicsStage>()) {
      for (const auto* resource : g_stage->writes_) {
        if (const auto* texture = resource->as<TextureResource>()) {
          if (texture->resolve_) {
            resolves.push_back(texture->resolve_);
            g_stage->load_store_ops_.insert(std::make_pair(
              texture->resolve_, g_stage->load_store_ops_.at(texture)));
          }
        }
      }
      for (auto* texture : resolves) {
        g_stage->writes_.insert(texture);
        g_stage->resolves_.insert(texture);
      }
    }
  }

  // ---------------------------------------------------------------------------
  // Compilation/scheduling
  // ---------------------------------------------------------------------------
  // TODO: the topology sorting implemented here has not been extensively
  // tested, but it works for the two stages I'm currently using...

  // Make a copy of all reads to use when topology sorting to avoid
  // modifying the real graph
  std::unordered_map<RenderStage*, std::unordered_set<const RenderResource*>>
    reads;
  for (auto& stage : stages_) {
    reads[stage.get()] = stage->reads_;
  }

  // Set up the topology sort
  std::vector<RenderStage*> q_set{}; // stages with no read dependencies
  for (auto& kv : reads) {
    bool has_raw_dep{ false };
    for (const auto* resource : kv.second) {
      for (const auto& stage : stages_) {
        if (stage.get() == kv.first)
          continue;
        if (stage->writes_.contains(resource)) {
          has_raw_dep = true;
        }
      }
    }
    if (not has_raw_dep) {
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
  std::vector<std::vector<RenderStage*>> t_sorted_stack;
  while (!q_set.empty()) {
    t_sorted_stack.emplace_back(q_set);
    q_set.clear();
    for (auto* q_stage : t_sorted_stack.back()) {
      for (auto* write : q_stage->writes_) {
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
      Throw("Render graph contains cyclic dependencies.");
    }
  }

  // Schedule passes within each group. Stages that clear resources go first.
  for (const auto& v : t_sorted_stack) {
    std::deque<RenderStage*> group_schedule;
    for (size_t i{ 0 }; i < v.size(); ++i) {
      auto* stage = v[i]->as<GraphicsStage>();
      if (not stage) {
        group_schedule.push_back(v[i]);
        continue;
      }
      bool has_clear{ false };
      for (const auto* write : stage->writes_) {
        if (const auto* texture = write->as<TextureResource>()) {
          if (stage->load_store_ops_.at(texture).first ==
              VK_ATTACHMENT_LOAD_OP_CLEAR) {
            has_clear = true;
            break;
          }
        }
      }
      if (has_clear) {
        group_schedule.push_front(stage);
      }
      else {
        group_schedule.push_back(stage);
      }
    }
    stage_stack_.emplace_back(group_schedule.begin(), group_schedule.end());
  }

#ifdef DEBUG
  std::ostringstream ss;
  for (size_t i = 0; i < stage_stack_.size(); ++i) {
    ss << "  - Stage group " << i + 1 << ": [ ";
    for (size_t j = 0; j < stage_stack_[i].size() - 1; ++j) {
      ss << stage_stack_[i][j]->name_ + ", ";
    }
    ss << stage_stack_[i].back()->name_ << " ]\n";
  }
  Log(Debug, "Proposed stage order:\n{}", ss.str());
#endif

  Log(Trace, "Allocating physical resource for buffers:");
  for (auto& buffer_resource : buffer_resources_) {
    Log(Trace, "  - {}", buffer_resource->name_);
    buffer_resource->physical_ = std::make_unique<PhysicalBuffer>();
  }

  Log(Trace, "Allocating physical resource for texture:");
  for (auto& texture : texture_resources_) {
    Log(Trace, "  - {}", texture->name_);

    // if (texture_resource->usage_ == TextureUsage::BackBuffer) {
    //   texture_resource->physical_ = std::make_shared<PhysicalBackBuffer>();
    //   continue;
    // }
    VkImageUsageFlags  usage_flags;
    VkImageAspectFlags aspect_flags;
    VkImageLayout      layout;
    switch (texture->usage_) {
      case TextureUsage::Color:
        usage_flags  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        break;
      case TextureUsage::DepthStencil:
        usage_flags  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
        layout       = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        break;
      default:
        Throw("Image creation for this usage has not been implemented yet.");
        break;
    }
    if (texture.get() == backBuffer()) {
      usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // to copy to swapchain
    }

    const wr::ImageCreateInfo texture_info{
      .name         = fmt::format("{} image", texture->name_),
      .extent       = swapchain_.extent(),
      .format       = texture->format_,
      .tiling       = VK_IMAGE_TILING_OPTIMAL,
      .usage_flags  = usage_flags,
      .aspect_flags = aspect_flags,
      .sample_count = texture->sample_count_,
      .mip_levels   = 1,
      .memory_usage = VMA_MEMORY_USAGE_GPU_ONLY,
      .final_layout = layout,
    };

    auto physical      = std::make_unique<PhysicalImage>();
    physical->image_   = wr::Image{ device_, texture_info };
    texture->physical_ = std::move(physical);
  }

  for (auto substages : stage_stack_) {
    for (auto* stage : substages) {
      if (auto* graphics_stage = stage->as<GraphicsStage>()) {
        auto g_stage_ptr = std::make_unique<PhysicalGraphicsStage>();
        // Need PhysicalGraphicsStage& later
        auto& physical            = *g_stage_ptr;
        graphics_stage->physical_ = std::move(g_stage_ptr); // upcasts

        //// Deduce sample count to be used in stage
        // VkSampleCountFlagBits sample_count{ VK_SAMPLE_COUNT_1_BIT };
        // for (auto* resource : stage->writes_) {
        //   if (const auto* texture = resource->as<TextureResource>()) {
        //     if (texture->sample_count_ > sample_count) {
        //       sample_count = texture->sample_count_;
        //     }
        //   }
        // }
        // graphics_stage->sample_count_ = sample_count;
        // log_->debug("Stage '{}' sample count: {}",
        //             graphics_stage->name_,
        //             static_cast<uint32_t>(sample_count));

        buildAttachments(graphics_stage, physical);
        // buildRenderPass(graphics_stage, physical);
        // buildPipelineLayout(graphics_stage, physical);
        // buildGraphicsPipeline(graphics_stage, physical);
      }
    }
  }
}

void RenderGraph::render(const wr::CommandBuffer& cb, wr::Image& target)
{
  Assert(target.layout() == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         "Render graph expects target to be in "
         "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL");
  auto* bb = back_buffer_->physical_->as<PhysicalImage>();
  Assert(bb);
  const VkExtent2D size{ bb->image_.size() };
  Assert(size.width == target.size().width and
           size.height == target.size().height,
         "Target must be of same size as render graph output");

  for (auto& buffer_resource : buffer_resources_) {
    if (buffer_resource->data_.data() != nullptr) {
      // There is data to be uploaded to the gpu
      const size_t data_size{ buffer_resource->data_.size_bytes() };
      auto*        physical = buffer_resource->physical_->as<PhysicalBuffer>();
      Assert(physical);
      if (data_size == 0) {
        // Free the buffer
        // TODO: Decide whether BufferResource::uploadData() should allow
        // upploading empty spans at all. Doing so is most likely a
        // mistake (as far as I can see right now).
        // physical.buffer_.reset();
        physical->buffer_ = {};
      }
      else {
        bool new_buffer_needed = false;
        if (unlikely(physical->buffer_.empty())) {
          new_buffer_needed = true;
        }
        else {
          // A gpu buffer already exists
          if (data_size != physical->buffer_.sizeBytes()) {
            // The gpu buffer needs to be resized
            new_buffer_needed = true;
          }
        }

        if (new_buffer_needed) {
          // Otherwise build a new GPU buffer
          physical->buffer_ = { buffer_resource->name_,
                                device_,
                                data_size,
                                buffer_resource->buffer_usage_ };
        }
      }
      // Upload data
      // TODO: GPU only buffers (use staging buffer)
      physical->buffer_.uploadData(buffer_resource->data_);
      // Reset data pointer once it has been uploaded to gpu
      buffer_resource->data_ = {};
    }
  }

  // TODO: full memory barrier is not needed between nodes in same subset
  for (const auto& subset : stage_stack_) {
    for (const auto& stage : subset) {
      recordCommandBuffer(stage, cb);
    }
  }

  // Copy back buffer to target
  const VkImageCopy2 regions[] {{
    .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
    .pNext = {},
    .srcSubresource = {
      .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel       = 0,
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
    .srcOffset = {0,0,0},
    .dstSubresource = {
      .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel       = 0,
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
    .dstOffset = {0,0,0},
    .extent = {size.width, size.height, 1}
  }};
  cb.transitionImageLayout(bb->image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    .copyImage(target, bb->image_, regions)
    .transitionImageLayout(bb->image_,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

NAMESPACE_END(eldr::vk)
