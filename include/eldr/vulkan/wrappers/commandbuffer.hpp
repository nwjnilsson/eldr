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
  // const CommandBuffer&   beginSingleCommand() const;
  const CommandBuffer& drawIndexed(uint32_t index_count,
                                   uint32_t instance_count = 1,
                                   uint32_t first_index    = 0,
                                   int32_t  vertex_offset  = 0,
                                   uint32_t first_instance = 0) const;
  const CommandBuffer& endRenderPass() const;
  const CommandBuffer& end() const;
  const CommandBuffer& submit(const VkSubmitInfo& submit_info) const;
  const CommandBuffer& submitAndWait(const VkSubmitInfo& submit_info) const;
  const CommandBuffer& submitAndWait() const;
  const CommandBuffer& submit() const;
  const CommandBuffer& reset() const;
  const CommandBuffer&
  bindIndexBuffer(const GpuBuffer&,
                  VkIndexType  index_type = VK_INDEX_TYPE_UINT32,
                  VkDeviceSize offset     = 0) const;

  const CommandBuffer&
  bindVertexBuffers(std::span<const VkBuffer>, uint32_t first_binding = 0,
                    std::span<const VkDeviceSize> offsets = {}) const;

  const CommandBuffer& bindDescriptorSets(
    std::span<const VkDescriptorSet> desc_sets, VkPipelineLayout layout,
    VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
    uint32_t first_set = 0, std::span<const uint32_t> dyn_offsets = {}) const;

  const CommandBuffer& bindPipeline(
    VkPipeline          pipeline,
    VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

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
  const CommandBuffer& setViewport(std::span<const VkViewport> viewports,
                                   uint32_t first_viewport) const;
  const CommandBuffer& setScissor(std::span<const VkRect2D> scissors,
                                  uint32_t first_scissor) const;
  const CommandBuffer& fullBarrier() const;

  const CommandBuffer& transitionImageLayout(const GpuImage&,
                                             VkImageLayout old_layout,
                                             VkImageLayout new_layout,
                                             uint32_t      mip_levels) const;

  const CommandBuffer&
  blitImage(const GpuImage& src_image, VkImageLayout src_layout,
            const GpuImage& dst_image, VkImageLayout dst_layout,
            const VkImageBlit& blit, VkFilter filter) const;

  const CommandBuffer& generateMipmaps(const GpuImage& image,
                                       uint32_t        mip_levels) const;

  const CommandBuffer& copyBufferToImage(VkBuffer buffer, VkImage image,
                                         const VkBufferImageCopy&) const;
  const CommandBuffer& copyBufferToImage(const void* data, VkDeviceSize,
                                         VkImage, const VkBufferImageCopy&,
                                         const std::string& name) const;

  const CommandBuffer& pushConstants(VkPipelineLayout   layout,
                                     VkShaderStageFlags stage, uint32_t size,
                                     const void*  data,
                                     VkDeviceSize offset = 0) const;
  template <typename T>
  const CommandBuffer& pushConstant(const VkPipelineLayout   layout,
                                    const T&                 data,
                                    const VkShaderStageFlags stage,
                                    const VkDeviceSize       offset = 0) const
  {
    return pushConstants(layout, stage, sizeof(data), &data, offset);
  }

  [[nodiscard]] VkBuffer createStagingBuffer(const void*        data,
                                             VkDeviceSize       buffer_size,
                                             const std::string& name) const;

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkResult           fenceStatus() const;
  void                             resetFence() const;
  void                             waitFence() const;

private:
  const Device&      device_;
  const CommandPool& command_pool_;
  std::string        name_;

  VkCommandBuffer                command_buffer_{ VK_NULL_HANDLE };
  std::unique_ptr<Fence>         wait_fence_;
  mutable std::vector<GpuBuffer> staging_buffers_;
};
} // namespace eldr::vk::wr
