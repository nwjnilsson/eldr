#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/commandpool.hpp>

#include <span>
#include <vector>

namespace eldr::vk::wr {

class CommandBuffer {
public:
  CommandBuffer() = delete;
  CommandBuffer(const Device&      device_, const CommandPool&,
                const std::string& name);
  CommandBuffer(CommandBuffer&&) noexcept;
  CommandBuffer(const CommandBuffer&) = delete;
  ~CommandBuffer();

  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer& operator=(CommandBuffer&&)      = delete;

  const VkCommandBuffer& get() const { return command_buffer_; }
  const CommandBuffer&   begin(VkCommandBufferUsageFlags usage = 0) const;
  const CommandBuffer&   beginRenderPass(const VkRenderPassBeginInfo&) const;
  const CommandBuffer&   beginSingleCommand() const;
  const CommandBuffer&   endRenderPass() const;
  const CommandBuffer&   end() const;
  const CommandBuffer&   submit() const;
  const CommandBuffer&   reset() const;
  const CommandBuffer&   bindIndexBuffer(const wr::GpuBuffer&) const;
  const CommandBuffer& bindVertexBuffers(const std::span<wr::GpuBuffer*>) const;
  const CommandBuffer& bindPipeline(const wr::Pipeline&) const;
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

  const CommandBuffer& transitionImageLayout(const GpuImage&,
                                             VkImageLayout old_layout,
                                             VkImageLayout new_layout) const;

  const CommandBuffer&
  blitImage(const GpuImage& src_image, VkImageLayout src_layout,
            const GpuImage& dst_image, VkImageLayout dst_layout,
            const VkImageBlit& blit, VkFilter filter) const;

  const CommandBuffer& generateMipmaps(const GpuImage& image) const;

  const CommandBuffer& copyBufferToImage(VkBuffer buffer, VkImage image,
                                         const VkBufferImageCopy&) const;
  const CommandBuffer& copyBufferToImage(const void* data, VkDeviceSize,
                                         VkImage, const VkBufferImageCopy&,
                                         const std::string& name) const;

  [[nodiscard]] VkBuffer createStagingBuffer(const void*        data,
                                             VkDeviceSize       buffer_size,
                                             const std::string& name) const;

  [[nodiscard]] VkResult fenceStatus() const;
  void                   resetFence() const;

private:
  const Device&      device_;
  const CommandPool& command_pool_;
  std::string        name_;

  VkCommandBuffer                command_buffer_{ VK_NULL_HANDLE };
  std::unique_ptr<Fence>         wait_fence_;
  mutable std::vector<GpuBuffer> staging_buffers_;
};
} // namespace eldr::vk::wr
