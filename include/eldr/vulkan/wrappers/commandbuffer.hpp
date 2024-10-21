#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>

#include <span>

namespace eldr::vk::wr {

class CommandBuffer {
public:
  CommandBuffer() = delete;
  CommandBuffer(const Device& device_, const CommandPool&);
  ~CommandBuffer();
  CommandBuffer(CommandBuffer&&)      = delete;
  CommandBuffer(const CommandBuffer&) = delete;

  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer& operator=(CommandBuffer&&)      = delete;

  const VkCommandBuffer& get() const { return command_buffer_; }
  void                   begin(VkCommandBufferUsageFlags usage = 0) const;
  void                   beginSingleCommand() const;
  void                   submit() const;
  void                   beginRenderPass(const VkRenderPassBeginInfo&) const;
  void                   bindIndexBuffer(const wr::Buffer&) const;
  void                   bindVertexBuffers(const std::span<wr::Buffer*>) const;
  void                   bindPipeline(const wr::Pipeline&) const;
  void                   endRenderPass() const;
  const CommandBuffer&
  pipelineBarrier(VkPipelineStageFlags                   src_stage_flags,
                  VkPipelineStageFlags                   dst_stage_flags,
                  std::span<const VkImageMemoryBarrier>  img_mem_barriers,
                  std::span<const VkMemoryBarrier>       mem_barriers     = {},
                  std::span<const VkBufferMemoryBarrier> buf_mem_barriers = {},
                  VkDependencyFlags                      dep_flags = 0) const;
  const CommandBuffer&
  pipelineImageMemoryBarrier(VkPipelineStageFlags        src_stage_flags,
                             VkPipelineStageFlags        dst_stage_flags,
                             const VkImageMemoryBarrier& barrier) const;
  const CommandBuffer&
  pipelineMemoryBarrier(VkPipelineStageFlags   src_stage_flags,
                        VkPipelineStageFlags   dst_stage_flags,
                        const VkMemoryBarrier& barrier) const;
  const CommandBuffer& fullBarrier() const;
  void                 end() const;
  void                 reset() const;

private:
  const Device&      device_;
  const CommandPool& command_pool_;

  VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
