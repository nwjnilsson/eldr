#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>
#include <eldr/vulkan/wrappers/device.hpp>

namespace eldr::vk::wr {
CommandBuffer::CommandBuffer(const Device&      device,
                             const CommandPool& command_pool)
  : device_(device), command_pool_(command_pool)
{
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_.get();
  alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  CheckVkResult(
    vkAllocateCommandBuffers(device_.logical(), &alloc_info, &command_buffer_));
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

void CommandBuffer::begin(VkCommandBufferUsageFlags usage) const
{
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags            = usage;
  begin_info.pInheritanceInfo = nullptr;
  CheckVkResult(vkBeginCommandBuffer(command_buffer_, &begin_info));
}

void CommandBuffer::beginSingleCommand() const
{
  begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void CommandBuffer::beginRenderPass(
  const VkRenderPassBeginInfo& render_pass_info) const
{
  vkCmdBeginRenderPass(command_buffer_, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass() const
{
  vkCmdEndRenderPass(command_buffer_);
}

void CommandBuffer::end() const
{
  CheckVkResult(vkEndCommandBuffer(command_buffer_));
}

// TODO: may want to optimize this function if used often (use fences instead of
// vkQueueWaitIdle)
void CommandBuffer::submit() const
{
  auto submit_info               = makeInfo<VkSubmitInfo>();
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers    = &command_buffer_;

  VkResult res;
  res = vkQueueSubmit(device_.graphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
  CheckVkResult(res);
  res = vkQueueWaitIdle(device_.graphicsQueue());
  CheckVkResult(res);
}

void CommandBuffer::reset() const
{
  CheckVkResult(vkResetCommandBuffer(command_buffer_, 0));
}

} // namespace eldr::vk::wr
