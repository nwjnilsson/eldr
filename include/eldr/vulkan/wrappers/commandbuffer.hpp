#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

#include <span>
#include <vector>

namespace eldr::vk::wr {

class CommandBuffer {
  using byte = std::byte;

public:
  CommandBuffer() = default;
  CommandBuffer(const Device&      device_,
                const CommandPool& command_pool,
                std::string_view   name);

  const CommandBuffer& begin(VkCommandBufferUsageFlags usage = 0) const;
  const CommandBuffer& beginRenderPass(const VkRenderPassBeginInfo&) const;
  const CommandBuffer& beginRendering(const VkRenderingInfoKHR&) const;
  // const CommandBuffer&   beginSingleCommand() const;
  const CommandBuffer& drawIndexed(uint32_t index_count,
                                   uint32_t instance_count = 1,
                                   uint32_t first_index    = 0,
                                   int32_t  vertex_offset  = 0,
                                   uint32_t first_instance = 0) const;
  const CommandBuffer& endRenderPass() const;
  const CommandBuffer& endRendering() const;
  const CommandBuffer& end() const;
  const CommandBuffer& submit(const VkSubmitInfo& submit_info) const;
  const CommandBuffer& submitAndWait(const VkSubmitInfo& submit_info) const;
  const CommandBuffer& submitAndWait() const;
  const CommandBuffer& submit() const;
  const CommandBuffer& reset() const;

  const CommandBuffer&
  bindIndexBuffer(const GpuBuffer<byte>& buffer,
                  VkIndexType            index_type = VK_INDEX_TYPE_UINT32,
                  VkDeviceSize           offset     = 0) const;

  /// @brief Bind vertex buffers. Note that the type is VkBuffer here and not
  /// GpuBuffer, because the underlying call to vkCmdBindVertexBuffers expects
  /// an array of VkBuffer.
  const CommandBuffer&
  bindVertexBuffers(std::span<const VkBuffer>,
                    uint32_t                      first_binding = 0,
                    std::span<const VkDeviceSize> offsets       = {}) const;

  const CommandBuffer& bindDescriptorSets(
    std::span<const VkDescriptorSet> desc_sets,
    VkPipelineLayout                 layout,
    VkPipelineBindPoint       bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
    uint32_t                  first_set   = 0,
    std::span<const uint32_t> dyn_offsets = {}) const;

  const CommandBuffer& bindPipeline(
    const Pipeline&     pipeline,
    VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

  const CommandBuffer& pipelineBarrier(
    std::span<const VkImageMemoryBarrier2>  img_mem_barriers,
    std::span<const VkMemoryBarrier2>       mem_barriers,
    std::span<const VkBufferMemoryBarrier2> buf_mem_barriers) const;

  const CommandBuffer&
  pipelineImageMemoryBarrier(const VkImageMemoryBarrier2& barrier) const;
  const CommandBuffer&
  pipelineMemoryBarrier(const VkMemoryBarrier2& barrier) const;
  const CommandBuffer&
  pipelineBufferMemoryBarrier(const VkBufferMemoryBarrier2& barrier) const;

  const CommandBuffer& setViewport(std::span<const VkViewport> viewports,
                                   uint32_t first_viewport) const;
  const CommandBuffer& setScissor(std::span<const VkRect2D> scissors,
                                  uint32_t first_scissor) const;
  const CommandBuffer& fullBarrier() const;

  const CommandBuffer& transitionImageLayout(Image&,
                                             VkImageLayout new_layout) const;

  const CommandBuffer& blitImage(const Image&       src_image,
                                 VkImageLayout      src_layout,
                                 const Image&       dst_image,
                                 VkImageLayout      dst_layout,
                                 const VkImageBlit& blit,
                                 VkFilter           filter) const;

  const CommandBuffer& generateMipmaps(const Image& image) const;

  template <typename T>
  const CommandBuffer& copyBufferToImage(const GpuBuffer<T>& buffer,
                                         Image&              image,
                                         const VkBufferImageCopy&) const;

  const CommandBuffer& copyDataToImage(std::span<const byte> data,
                                       Image&                image,
                                       const VkBufferImageCopy&) const;

  const CommandBuffer& pushConstants(VkPipelineLayout   layout,
                                     VkShaderStageFlags stage,
                                     uint32_t           size,
                                     const void*        data,
                                     VkDeviceSize       offset = 0) const;
  template <typename T>
  const CommandBuffer& pushConstant(const VkPipelineLayout   layout,
                                    const T&                 data,
                                    const VkShaderStageFlags stage,
                                    const VkDeviceSize       offset = 0) const
  {
    return pushConstants(layout, stage, sizeof(data), &data, offset);
  }

  [[nodiscard]] const GpuBuffer<byte>&
  createStagingBuffer(std::string_view name, std::span<const byte> data) const;

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkCommandBuffer    get() const;
  [[nodiscard]] VkCommandBuffer*   ptr() const;
  [[nodiscard]] VkResult           fenceStatus() const;
  void                             resetFence() const;
  void                             waitFence() const;

private:
  std::string name_;

  class CommandBufferImpl;
  std::shared_ptr<CommandBufferImpl> d_;

  Fence                                wait_fence_;
  mutable std::vector<GpuBuffer<byte>> staging_buffers_;
};
} // namespace eldr::vk::wr
