#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// CommandBufferImpl
//------------------------------------------------------------------------------
class CommandBuffer::CommandBufferImpl {
public:
  CommandBufferImpl(const Device&                     device,
                    const VkCommandBufferAllocateInfo alloc_info);
  ~CommandBufferImpl();
  const Device    device_;
  VkCommandPool   command_pool_;
  VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };
};

CommandBuffer::CommandBufferImpl::CommandBufferImpl(
  const Device& device, const VkCommandBufferAllocateInfo alloc_info)
  : device_(device), command_pool_(alloc_info.commandPool)
{
  if (const VkResult result{ vkAllocateCommandBuffers(
        device_.logical(), &alloc_info, &command_buffer_) };
      result != VK_SUCCESS)
    Throw("Failed to allocate command buffers ({})", result);
}

CommandBuffer::CommandBufferImpl::~CommandBufferImpl()
{
  vkFreeCommandBuffers(device_.logical(), command_pool_, 1, &command_buffer_);
}

//------------------------------------------------------------------------------
// CommandBuffer
//------------------------------------------------------------------------------
CommandBuffer::CommandBuffer(const Device&      device,
                             const CommandPool& command_pool,
                             std::string_view   name)
  : name_(name), wait_fence_(device)
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool.vk();
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  d_ = std::make_shared<CommandBufferImpl>(device, alloc_info);
}

const CommandBuffer& CommandBuffer::pipelineBarrier(
  const std::span<const VkImageMemoryBarrier2>  img_mem_barriers,
  const std::span<const VkMemoryBarrier2>       mem_barriers,
  const std::span<const VkBufferMemoryBarrier2> buf_mem_barriers) const
{
  // One barrier must be set at least
  Assert(!(img_mem_barriers.empty() && mem_barriers.empty()) &&
         buf_mem_barriers.empty());

  VkDependencyInfo dep_info{
    .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .pNext                    = {},
    .dependencyFlags          = {},
    .memoryBarrierCount       = static_cast<uint32_t>(mem_barriers.size()),
    .pMemoryBarriers          = mem_barriers.data(),
    .bufferMemoryBarrierCount = static_cast<uint32_t>(buf_mem_barriers.size()),
    .pBufferMemoryBarriers    = buf_mem_barriers.data(),
    .imageMemoryBarrierCount  = static_cast<uint32_t>(img_mem_barriers.size()),
    .pImageMemoryBarriers     = img_mem_barriers.data(),
  };

  vkCmdPipelineBarrier2(d_->command_buffer_, &dep_info);
  return *this;
}

const CommandBuffer& CommandBuffer::pipelineImageMemoryBarrier(
  const VkImageMemoryBarrier2& img_barrier) const
{
  return pipelineBarrier({ &img_barrier, 1 }, {}, {});
}

const CommandBuffer&
CommandBuffer::pipelineMemoryBarrier(const VkMemoryBarrier2& mem_barrier) const
{
  return pipelineBarrier({}, { &mem_barrier, 1 }, {});
}

const CommandBuffer& CommandBuffer::pipelineBufferMemoryBarrier(
  const VkBufferMemoryBarrier2& buff_mem_barrier) const
{
  return pipelineBarrier({}, {}, { &buff_mem_barrier, 1 });
}

const CommandBuffer&
CommandBuffer::setViewport(std::span<const VkViewport> viewports,
                           uint32_t                    first_viewport) const
{
  vkCmdSetViewport(
    d_->command_buffer_, first_viewport, viewports.size(), viewports.data());
  return *this;
}

const CommandBuffer&
CommandBuffer::setScissor(std::span<const VkRect2D> scissors,
                          uint32_t                  first_scissor) const
{
  vkCmdSetScissor(
    d_->command_buffer_, first_scissor, scissors.size(), scissors.data());
  return *this;
}

const CommandBuffer& CommandBuffer::fullBarrier() const
{
  const VkMemoryBarrier2 barrier{
    .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
    .pNext         = {},
    .srcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
    .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
    .dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
    .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT
  };
  return pipelineMemoryBarrier(barrier);
}

const CommandBuffer& CommandBuffer::begin(VkCommandBufferUsageFlags usage) const
{
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags            = usage;
  begin_info.pInheritanceInfo = nullptr;
  if (const VkResult result{
        vkBeginCommandBuffer(d_->command_buffer_, &begin_info) };
      result != VK_SUCCESS)
    Throw("Failed to begin command buffer ({})!", result);
  return *this;
}

const CommandBuffer& CommandBuffer::beginRenderPass(
  const VkRenderPassBeginInfo& render_pass_info) const
{
  vkCmdBeginRenderPass(
    d_->command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
  return *this;
}

const CommandBuffer&
CommandBuffer::beginRendering(const VkRenderingInfoKHR& render_info) const
{
  vkCmdBeginRendering(d_->command_buffer_, &render_info);
  return *this;
}

const CommandBuffer& CommandBuffer::drawIndexed(uint32_t index_count,
                                                uint32_t inst_count,
                                                uint32_t first_index,
                                                int32_t  vert_offset,
                                                uint32_t first_inst) const
{
  vkCmdDrawIndexed(d_->command_buffer_,
                   index_count,
                   inst_count,
                   first_index,
                   vert_offset,
                   first_inst);
  return *this;
}

const CommandBuffer& CommandBuffer::endRenderPass() const
{
  vkCmdEndRenderPass(d_->command_buffer_);
  return *this;
}

const CommandBuffer& CommandBuffer::endRendering() const
{
  vkCmdEndRendering(d_->command_buffer_);
  return *this;
}

const CommandBuffer& CommandBuffer::end() const
{
  if (const VkResult result{ vkEndCommandBuffer(d_->command_buffer_) };
      result != VK_SUCCESS)
    Throw("Failed to end command buffer ({})!", result);
  return *this;
}

const CommandBuffer&
CommandBuffer::submit(const VkSubmitInfo& submit_info) const
{
  end();
  if (const VkResult result{ vkQueueSubmit(
        d_->device_.graphicsQueue(), 1, &submit_info, wait_fence_.vk()) };
      result != VK_SUCCESS)
    Throw("Failed to submit to queue ({})!", result);
  return *this;
}

const CommandBuffer&
CommandBuffer::submitAndWait(const VkSubmitInfo& submit_info) const
{
  submit(submit_info);
  waitFence();
  return *this;
}

const CommandBuffer& CommandBuffer::submitAndWait() const
{
  const VkSubmitInfo submit_info{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = {},
    .waitSemaphoreCount   = 0,
    .pWaitSemaphores      = {},
    .pWaitDstStageMask    = {},
    .commandBufferCount   = 1,
    .pCommandBuffers      = &d_->command_buffer_,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores    = {},
  };
  return submitAndWait(submit_info);
}

const CommandBuffer& CommandBuffer::reset() const
{
  if (const VkResult result{ vkResetCommandBuffer(d_->command_buffer_, 0) };
      result != VK_SUCCESS)
    Throw("Failed to reset command buffer ({})", result);
  return *this;
}

const CommandBuffer& CommandBuffer::blitImage(const Image&       src_image,
                                              VkImageLayout      src_layout,
                                              const Image&       dst_image,
                                              VkImageLayout      dst_layout,
                                              const VkImageBlit& blit,
                                              VkFilter           filter) const
{
  vkCmdBlitImage(d_->command_buffer_,
                 src_image.vk(),
                 src_layout,
                 dst_image.vk(),
                 dst_layout,
                 1,
                 &blit,
                 filter);
  return *this;
}

const CommandBuffer&
CommandBuffer::bindIndexBuffer(const Buffer<uint32_t>& buffer,
                               VkIndexType             index_type,
                               VkDeviceSize            offset) const
{
  Assert(buffer.vk());
  vkCmdBindIndexBuffer(d_->command_buffer_, buffer.vk(), offset, index_type);
  return *this;
}

const CommandBuffer&
CommandBuffer::bindVertexBuffers(std::span<const VkBuffer>     buffers,
                                 uint32_t                      first_binding,
                                 std::span<const VkDeviceSize> offsets) const
{
  std::vector<VkDeviceSize> zero_offsets(buffers.size(), 0);
  const VkDeviceSize*       p_offsets{ zero_offsets.data() };
  Assert(!buffers.empty());
  if (!offsets.empty()) {
    Assert(buffers.size() == offsets.size());
    p_offsets = offsets.data();
  }
  vkCmdBindVertexBuffers(d_->command_buffer_,
                         first_binding,
                         static_cast<uint32_t>(buffers.size()),
                         buffers.data(),
                         p_offsets);
  return *this;
}

const CommandBuffer&
CommandBuffer::bindDescriptorSets(std::span<const VkDescriptorSet> desc_sets,
                                  VkPipelineLayout                 layout,
                                  VkPipelineBindPoint              bind_point,
                                  uint32_t                         first_set,
                                  std::span<const uint32_t> dyn_offsets) const
{
  Assert(layout);
  Assert(!desc_sets.empty());
  vkCmdBindDescriptorSets(d_->command_buffer_,
                          bind_point,
                          layout,
                          first_set,
                          static_cast<uint32_t>(desc_sets.size()),
                          desc_sets.data(),
                          static_cast<uint32_t>(dyn_offsets.size()),
                          dyn_offsets.data());
  return *this;
}

const CommandBuffer&
CommandBuffer::bindPipeline(const Pipeline&     pipeline,
                            VkPipelineBindPoint bind_point) const
{
  Assert(pipeline.vk());
  vkCmdBindPipeline(d_->command_buffer_, bind_point, pipeline.vk());
  return *this;
}

const CommandBuffer& CommandBuffer::generateMipmaps(const Image& image) const
{
  VkFormatProperties format_props{};
  vkGetPhysicalDeviceFormatProperties(
    d_->device_.physical(), image.format(), &format_props);
  // Support for linear blitting is currently required
  if (!(format_props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    Throw("Texture image format does not support linear blitting!");
  }

  VkImageMemoryBarrier2 barrier{
    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .pNext               = {},
    .srcStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    .srcAccessMask       = {},
    .dstStageMask        = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    .dstAccessMask       = {},
    .oldLayout           = {},
    .newLayout           = {},
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image               = image.vk(),
    .subresourceRange    = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel   = 0,
                             .levelCount     = 1,
                             .baseArrayLayer = 0,
                             .layerCount     = 1 },
  };

  int32_t mip_width  = image.size().width;
  int32_t mip_height = image.size().height;

  for (uint32_t i = 1; i < image.mipLevels(); ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
    pipelineImageMemoryBarrier(barrier);

    const VkImageBlit blit{
      .srcSubresource = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                          .mipLevel       = i - 1,
                          .baseArrayLayer = 0,
                          .layerCount     = 1 },
      .srcOffsets     = { { 0, 0, 0 }, { mip_width, mip_height, 1 } },
      .dstSubresource = { .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                          .mipLevel       = i,
                          .baseArrayLayer = 0,
                          .layerCount     = 1 },
      .dstOffsets     = { { 0, 0, 0 },
                          { mip_width > 1 ? mip_width / 2 : 1,
                        mip_height > 1 ? mip_height / 2 : 1,
                            1 } },
    };
    blitImage(image,
              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
              image,
              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
              blit,
              VK_FILTER_LINEAR);

    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    pipelineImageMemoryBarrier(barrier);

    if (mip_width > 1)
      mip_width /= 2;
    if (mip_height > 1)
      mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = image.mipLevels() - 1;
  barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
  barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
  barrier.dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

  return pipelineImageMemoryBarrier(barrier);
}

const CommandBuffer&
CommandBuffer::transitionImageLayout(Image&        image,
                                     VkImageLayout new_layout) const
{
  VkImageMemoryBarrier2 barrier{
    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .pNext               = {},
    .srcStageMask = {},
    .srcAccessMask       = 0,
    .dstStageMask = {},
    .dstAccessMask       = 0,
    .oldLayout           = image.layout(),
    .newLayout           = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image               = image.vk(),
    .subresourceRange = {
      .aspectMask     = image.view().aspectFlags(),
      .baseMipLevel   = 0,
      .levelCount     = image.mipLevels(),
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
  };

  if (image.layout() == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
  }
  else if (image.layout() == VK_IMAGE_LAYOUT_UNDEFINED &&
           new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
  else if (image.layout() == VK_IMAGE_LAYOUT_UNDEFINED &&
           new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                           VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
  }
  else if (image.layout() == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
           new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    barrier.srcStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
  }
  else {
    Throw(
      "Unsupported layout transition! ({} -> {})", image.layout(), new_layout);
  }

  image.setLayout(new_layout);
  return pipelineImageMemoryBarrier(barrier);
}

template <typename T>
const CommandBuffer&
CommandBuffer::copyBufferToImage(const Buffer<T>&         buffer,
                                 Image&                   image,
                                 const VkBufferImageCopy& copy_region) const
{
  vkCmdCopyBufferToImage(d_->command_buffer_,
                         buffer.vk(),
                         image.vk(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         1,
                         &copy_region);
  return *this;
}

const CommandBuffer&
CommandBuffer::copyDataToImage(std::span<const byte_t>  data,
                               Image&                   image,
                               const VkBufferImageCopy& copy_region) const
{
  return copyBufferToImage(
    createStagingBuffer(
      fmt::format("Staging buffer #{}", staging_buffers_.size() + 1), data),
    image,
    copy_region);
}

const CommandBuffer& CommandBuffer::pushConstants(VkPipelineLayout   layout,
                                                  VkShaderStageFlags stage,
                                                  uint32_t           size,
                                                  const void*        data,
                                                  VkDeviceSize offset) const
{
  Assert(layout);
  Assert(size > 0);
  Assert(data);
  vkCmdPushConstants(d_->command_buffer_,
                     layout,
                     stage,
                     static_cast<uint32_t>(offset),
                     size,
                     data);
  return *this;
}

const Buffer<byte_t>&
CommandBuffer::createStagingBuffer(const std::string&      name,
                                   std::span<const byte_t> data) const
{
  staging_buffers_.emplace_back(d_->device_,
                                name,
                                data,
                                +BufferUsage::TransferSrc,
                                +HostAccess::Sequential);
  return staging_buffers_.back();
}

VkResult CommandBuffer::fenceStatus() const { return wait_fence_.status(); }
void     CommandBuffer::waitFence() const
{
  // 5 seconds, vulkan timeout is in nanoseconds
  constexpr const uint64_t timeout = 5e9;
  if (wait_fence_.wait(timeout) == VK_TIMEOUT)
    Log(core::Debug, "{} timed out waiting for its internal fence!", name_);
}

void CommandBuffer::resetFence() const { wait_fence_.reset(); }

VkCommandBuffer CommandBuffer::vk() const { return d_->command_buffer_; }

VkCommandBuffer* CommandBuffer::vkp() const { return &d_->command_buffer_; }
} // namespace eldr::vk::wr
