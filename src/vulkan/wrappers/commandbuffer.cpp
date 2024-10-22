#include "eldr/vulkan/common.hpp"
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

namespace eldr::vk::wr {
CommandBuffer::CommandBuffer(const Device&      device,
                             const CommandPool& command_pool,
                             const std::string& name)
  : device_(device), command_pool_(command_pool), name_(name)
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_.get();
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  if (const auto result = vkAllocateCommandBuffers(
        device_.logical(), &alloc_info, &command_buffer_);
      result != VK_SUCCESS)
    ThrowVk(result,
            "vkAllocateCommandBuffers(): failed to allocate command buffers!");
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
  : device_(other.device_), command_pool_(other.command_pool_),
    name_(std::move(other.name_))
{
  command_buffer_  = std::exchange(other.command_buffer_, VK_NULL_HANDLE);
  wait_fence_      = std::move(other.wait_fence_);
  staging_buffers_ = std::move(other.staging_buffers_);
}

CommandBuffer::~CommandBuffer()
{
  if (command_buffer_ != VK_NULL_HANDLE)
    vkFreeCommandBuffers(device_.logical(), command_pool_.get(), 1,
                         &command_buffer_);
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

  vkCmdPipelineBarrier(
    command_buffer_, src_stage_flags, dst_stage_flags, dep_flags,
    static_cast<std::uint32_t>(mem_barriers.size()), mem_barriers.data(),
    static_cast<std::uint32_t>(buf_mem_barriers.size()),
    buf_mem_barriers.data(),
    static_cast<std::uint32_t>(img_mem_barriers.size()),
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
  return pipelineBarrier(src_stage_flags, dst_stage_flags, {},
                         { &mem_barrier, 1 });
}
const CommandBuffer& CommandBuffer::fullBarrier() const
{
  const VkMemoryBarrier barrier{ .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                                 .pNext = {},
                                 .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
                                 .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT };
  return pipelineMemoryBarrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, barrier);
}

const CommandBuffer& CommandBuffer::begin(VkCommandBufferUsageFlags usage) const
{
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags            = usage;
  begin_info.pInheritanceInfo = nullptr;
  if (const auto result = vkBeginCommandBuffer(command_buffer_, &begin_info);
      result != VK_SUCCESS)
    ThrowVk(result, "vkBeginCommandBuffer(): failed to begin command buffer!");
  return *this;
}

const CommandBuffer& CommandBuffer::beginSingleCommand() const
{
  return begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

const CommandBuffer& CommandBuffer::beginRenderPass(
  const VkRenderPassBeginInfo& render_pass_info) const
{
  vkCmdBeginRenderPass(command_buffer_, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);
  return *this;
}

const CommandBuffer& CommandBuffer::endRenderPass() const
{
  vkCmdEndRenderPass(command_buffer_);
  return *this;
}

const CommandBuffer& CommandBuffer::end() const
{
  if (const auto result = vkEndCommandBuffer(command_buffer_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkEndCommandBuffer(): failed to end command buffer!");
  return *this;
}

// TODO: may want to optimize this function if used often (use fences instead of
// vkQueueWaitIdle)
const CommandBuffer& CommandBuffer::submit() const
{
  const VkSubmitInfo submit_info{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = {},
    .waitSemaphoreCount   = {},
    .pWaitSemaphores      = {},
    .pWaitDstStageMask    = {},
    .commandBufferCount   = 1,
    .pCommandBuffers      = &command_buffer_,
    .signalSemaphoreCount = {},
    .pSignalSemaphores    = {},
  };
  if (const auto result =
        vkQueueSubmit(device_.graphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
      result != VK_SUCCESS)
    ThrowVk(result, "vkQueueSubmit(): failed to submit command buffer!");
  if (const auto result = vkQueueWaitIdle(device_.graphicsQueue());
      result != VK_SUCCESS)
    ThrowVk(result, "vkQueueWaitIdle(): failed to wait for queue idle!");
  return *this;
}

const CommandBuffer& CommandBuffer::reset() const
{
  if (const auto result = vkResetCommandBuffer(command_buffer_, 0);
      result != VK_SUCCESS)
    ThrowVk(result, "vkResetCommandBuffer(): failed to reset command buffer!");
  return *this;
}

const CommandBuffer&
CommandBuffer::blitImage(const GpuImage& src_image, VkImageLayout src_layout,
                         const GpuImage& dst_image, VkImageLayout dst_layout,
                         const VkImageBlit& blit, VkFilter filter) const
{
  vkCmdBlitImage(command_buffer_, src_image.get(), src_layout, dst_image.get(),
                 dst_layout, 1, &blit, filter);
  return *this;
}

const CommandBuffer& CommandBuffer::generateMipmaps(const GpuImage& image) const
{
  VkFormatProperties format_props{};
  vkGetPhysicalDeviceFormatProperties(device_.physical(), image.format(),
                                      &format_props);
  // Support for linear blitting is currently required
  if (!(format_props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    ThrowVk({},
            "generateMipmaps(): texture image format does not support linear "
            "blitting!");
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
    pipelineImageMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

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
                        mip_height > 1 ? mip_height / 2 : 1, 1 } },
    };
    blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, blit, VK_FILTER_LINEAR);

    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    pipelineImageMemoryBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
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

const CommandBuffer&
CommandBuffer::transitionImageLayout(const GpuImage& image,
                                     VkImageLayout   old_layout,
                                     VkImageLayout   new_layout) const
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
    ThrowVk({}, "Unsupported layout transition!");
  }

  return pipelineImageMemoryBarrier(source_stage, destination_stage, barrier);
}

const CommandBuffer&
CommandBuffer::copyBufferToImage(VkBuffer buffer, VkImage image,
                                 const VkBufferImageCopy& copy_region) const
{
  vkCmdCopyBufferToImage(command_buffer_, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
  return *this;
}

const CommandBuffer& CommandBuffer::copyBufferToImage(
  const void* data, VkDeviceSize buffer_size, VkImage image,
  const VkBufferImageCopy& copy_region, const std::string& name) const
{

  return copyBufferToImage(createStagingBuffer(data, buffer_size, name), image,
                           copy_region);
}

VkBuffer CommandBuffer::createStagingBuffer(const void*        data,
                                            VkDeviceSize       buffer_size,
                                            const std::string& name) const
{
  staging_buffers_.emplace_back(device_, buffer_size, data, buffer_size,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VMA_MEMORY_USAGE_CPU_ONLY, name);
  return staging_buffers_.back().get();
}

VkResult CommandBuffer::fenceStatus() const { return wait_fence_->status(); }
void     CommandBuffer::resetFence() const { wait_fence_->reset(); }

} // namespace eldr::vk::wr
