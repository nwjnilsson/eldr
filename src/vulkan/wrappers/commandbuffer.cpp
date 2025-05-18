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
  if (const auto result = vkAllocateCommandBuffers(
        device_.logical(), &alloc_info, &command_buffer_);
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
  alloc_info.commandPool = command_pool.get();
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  cb_data_ = std::make_shared<CommandBufferImpl>(device, alloc_info);
}

const CommandBuffer& CommandBuffer::pipelineBarrier(
  const VkPipelineStageFlags                   src_stage_flags,
  const VkPipelineStageFlags                   dst_stage_flags,
  const std::span<const VkImageMemoryBarrier>  img_mem_barriers,
  const std::span<const VkMemoryBarrier>       mem_barriers,
  const std::span<const VkBufferMemoryBarrier> buf_mem_barriers,
  const VkDependencyFlags                      dep_flags) const
{
  // One barrier must be set at least
  assert(!(img_mem_barriers.empty() && mem_barriers.empty()) &&
         buf_mem_barriers.empty());

  vkCmdPipelineBarrier(cb_data_->command_buffer_,
                       src_stage_flags,
                       dst_stage_flags,
                       dep_flags,
                       static_cast<uint32_t>(mem_barriers.size()),
                       mem_barriers.data(),
                       static_cast<uint32_t>(buf_mem_barriers.size()),
                       buf_mem_barriers.data(),
                       static_cast<uint32_t>(img_mem_barriers.size()),
                       img_mem_barriers.data());
  return *this;
}

const CommandBuffer& CommandBuffer::pipelineImageMemoryBarrier(
  const VkPipelineStageFlags  src_stage_flags,
  const VkPipelineStageFlags  dst_stage_flags,
  const VkImageMemoryBarrier& img_barrier) const
{
  return pipelineBarrier(src_stage_flags, dst_stage_flags, { &img_barrier, 1 });
}

const CommandBuffer&
CommandBuffer::pipelineMemoryBarrier(VkPipelineStageFlags   src_stage_flags,
                                     VkPipelineStageFlags   dst_stage_flags,
                                     const VkMemoryBarrier& mem_barrier) const
{
  return pipelineBarrier(
    src_stage_flags, dst_stage_flags, {}, { &mem_barrier, 1 });
}

const CommandBuffer&
CommandBuffer::setViewport(std::span<const VkViewport> viewports,
                           uint32_t                    first_viewport) const
{
  vkCmdSetViewport(cb_data_->command_buffer_,
                   first_viewport,
                   viewports.size(),
                   viewports.data());
  return *this;
}

const CommandBuffer&
CommandBuffer::setScissor(std::span<const VkRect2D> scissors,
                          uint32_t                  first_scissor) const
{
  vkCmdSetScissor(
    cb_data_->command_buffer_, first_scissor, scissors.size(), scissors.data());
  return *this;
}

const CommandBuffer& CommandBuffer::fullBarrier() const
{
  const VkMemoryBarrier barrier{ .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                                 .pNext = {},
                                 .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
                                 .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT };
  return pipelineMemoryBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               barrier);
}

const CommandBuffer& CommandBuffer::begin(VkCommandBufferUsageFlags usage) const
{
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags            = usage;
  begin_info.pInheritanceInfo = nullptr;
  if (const auto result =
        vkBeginCommandBuffer(cb_data_->command_buffer_, &begin_info);
      result != VK_SUCCESS)
    Throw("Failed to begin command buffer ({})!", result);
  return *this;
}

const CommandBuffer& CommandBuffer::beginRenderPass(
  const VkRenderPassBeginInfo& render_pass_info) const
{
  vkCmdBeginRenderPass(
    cb_data_->command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
  return *this;
}

const CommandBuffer&
CommandBuffer::beginRendering(const VkRenderingInfoKHR& render_info) const
{
  vkCmdBeginRendering(cb_data_->command_buffer_, &render_info);
  return *this;
}

const CommandBuffer& CommandBuffer::drawIndexed(uint32_t index_count,
                                                uint32_t inst_count,
                                                uint32_t first_index,
                                                int32_t  vert_offset,
                                                uint32_t first_inst) const
{
  vkCmdDrawIndexed(cb_data_->command_buffer_,
                   index_count,
                   inst_count,
                   first_index,
                   vert_offset,
                   first_inst);
  return *this;
}

const CommandBuffer& CommandBuffer::endRenderPass() const
{
  vkCmdEndRenderPass(cb_data_->command_buffer_);
  return *this;
}

const CommandBuffer& CommandBuffer::endRendering() const
{
  vkCmdEndRendering(cb_data_->command_buffer_);
  return *this;
}

const CommandBuffer& CommandBuffer::end() const
{
  if (const auto result = vkEndCommandBuffer(cb_data_->command_buffer_);
      result != VK_SUCCESS)
    Throw("Failed to end command buffer ({})!", result);
  return *this;
}

const CommandBuffer&
CommandBuffer::submit(const VkSubmitInfo& submit_info) const
{
  end();
  if (const auto result = vkQueueSubmit(
        cb_data_->device_.graphicsQueue(), 1, &submit_info, wait_fence_.get());
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
    .pCommandBuffers      = &cb_data_->command_buffer_,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores    = {},
  };
  return submitAndWait(submit_info);
}

const CommandBuffer& CommandBuffer::reset() const
{
  if (const auto result = vkResetCommandBuffer(cb_data_->command_buffer_, 0);
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
  vkCmdBlitImage(cb_data_->command_buffer_,
                 src_image.get(),
                 src_layout,
                 dst_image.get(),
                 dst_layout,
                 1,
                 &blit,
                 filter);
  return *this;
}

const CommandBuffer& CommandBuffer::bindIndexBuffer(const Buffer<byte>& buffer,
                                                    VkIndexType  index_type,
                                                    VkDeviceSize offset) const
{
  assert(buffer.get());
  vkCmdBindIndexBuffer(
    cb_data_->command_buffer_, buffer.get(), offset, index_type);
  return *this;
}

const CommandBuffer&
CommandBuffer::bindVertexBuffers(std::span<const VkBuffer>     buffers,
                                 uint32_t                      first_binding,
                                 std::span<const VkDeviceSize> offsets) const
{
  std::vector<VkDeviceSize> zero_offsets(buffers.size(), 0);
  const VkDeviceSize*       p_offsets{ zero_offsets.data() };
  assert(!buffers.empty());
  if (!offsets.empty()) {
    assert(buffers.size() == offsets.size());
    p_offsets = offsets.data();
  }
  vkCmdBindVertexBuffers(cb_data_->command_buffer_,
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
  assert(layout);
  assert(!desc_sets.empty());
  vkCmdBindDescriptorSets(cb_data_->command_buffer_,
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
  assert(pipeline.get());
  vkCmdBindPipeline(cb_data_->command_buffer_, bind_point, pipeline.get());
  return *this;
}

const CommandBuffer& CommandBuffer::generateMipmaps(const Image& image) const
{
  VkFormatProperties format_props{};
  vkGetPhysicalDeviceFormatProperties(
    cb_data_->device_.physical(), image.format(), &format_props);
  // Support for linear blitting is currently required
  if (!(format_props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    Throw("Texture image format does not support linear blitting!");
  }

  VkImageMemoryBarrier barrier{};
  barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image                       = image.get();
  barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;
  barrier.subresourceRange.levelCount     = 1;

  int32_t mip_width  = image.size().width;
  int32_t mip_height = image.size().height;

  for (uint32_t i = 1; i < image.mipLevels(); ++i) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    pipelineImageMemoryBarrier(
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

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
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    pipelineImageMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               barrier);
    if (mip_width > 1)
      mip_width /= 2;
    if (mip_height > 1)
      mip_height /= 2;
  }

  barrier.subresourceRange.baseMipLevel = image.mipLevels() - 1;
  barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  return pipelineImageMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                    barrier);
}

const CommandBuffer& CommandBuffer::transitionImageLayout(
  const Image& image, VkImageLayout old_layout, VkImageLayout new_layout) const
{
  VkImageMemoryBarrier barrier{};
  barrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout                   = old_layout;
  barrier.newLayout                   = new_layout;
  barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                       = image.get();
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = image.mipLevels();
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    source_stage          = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage     = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
           new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    source_stage          = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else {
    ThrowVk(VkResult{}, "Unsupported layout transition!");
  }

  return pipelineImageMemoryBarrier(source_stage, destination_stage, barrier);
}

template <typename T>
const CommandBuffer&
CommandBuffer::copyBufferToImage(const Buffer<T>&         buffer,
                                 Image&                   image,
                                 const VkBufferImageCopy& copy_region) const
{
  vkCmdCopyBufferToImage(cb_data_->command_buffer_,
                         buffer.get(),
                         image.get(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         1,
                         &copy_region);
  return *this;
}

const CommandBuffer&
CommandBuffer::copyDataToImage(std::span<const byte>    data,
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
  assert(layout);
  assert(size > 0);
  assert(data);
  vkCmdPushConstants(cb_data_->command_buffer_,
                     layout,
                     stage,
                     static_cast<uint32_t>(offset),
                     size,
                     data);
  return *this;
}

const Buffer<std::byte>&
CommandBuffer::createStagingBuffer(std::string_view      name,
                                   std::span<const byte> data) const
{
  staging_buffers_.emplace_back(cb_data_->device_,
                                name,
                                data,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VMA_MEMORY_USAGE_CPU_ONLY);
  return staging_buffers_.back();
}

VkResult CommandBuffer::fenceStatus() const { return wait_fence_.status(); }
void     CommandBuffer::waitFence() const
{
  // 5 seconds, vulkan timeout is in nanoseconds
  constexpr const uint64_t timeout = 5e9;
  if (wait_fence_.wait(timeout) == VK_TIMEOUT)
    cb_data_->device_.logger()->debug(
      "{} timed out waiting for its internal fence!", name_);
}

void CommandBuffer::resetFence() const { wait_fence_.reset(); }

VkCommandBuffer CommandBuffer::get() const { return cb_data_->command_buffer_; }

VkCommandBuffer* CommandBuffer::ptr() const
{
  return &cb_data_->command_buffer_;
}
} // namespace eldr::vk::wr
