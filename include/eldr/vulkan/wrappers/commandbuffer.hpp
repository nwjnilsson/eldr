#pragma once
#include <eldr/vulkan/vulkan.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)

class CommandBuffer : public VkDeviceObject<VkCommandBuffer> {
  using Base = VkDeviceObject<VkCommandBuffer>;

public:
  EL_VK_IMPORT_DEFAULTS(CommandBuffer)
  CommandBuffer(std::string_view   name,
                const Device&      device_,
                const CommandPool& command_pool);

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
  /// @brief Submits and waits for commands to execute
  const CommandBuffer& submitAndWait() const;
  const CommandBuffer& submit() const;
  const CommandBuffer& reset() const;

  const CommandBuffer& bindIndexBuffer(const Buffer<uint32_t>& buffer,
                                       VkDeviceSize offset = 0) const;

  /// @brief Bind vertex buffers. Note that the type is VkBuffer here and not
  /// Buffer, because the underlying call to vkCmdBindVertexBuffers expects
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

  const CommandBuffer&
  transitionImageLayout(Image&,
                        VkImageLayout new_layout,
                        bool          force_layout_undefined = false) const;

  const CommandBuffer& blitImage(const Image&       src_image,
                                 VkImageLayout      src_layout,
                                 const Image&       dst_image,
                                 VkImageLayout      dst_layout,
                                 const VkImageBlit& blit,
                                 VkFilter           filter) const;

  const CommandBuffer& generateMipmaps(const Image& image) const;

  const CommandBuffer&
  copyImage(Image&                        dst,
            const Image&                  src,
            std::span<const VkImageCopy2> copy_regions) const;

  const CommandBuffer&
  copyBufferToImage(Image&                              dst,
                    const AllocatedBuffer&              src,
                    std::span<const VkBufferImageCopy2> copy_regions) const;

  const CommandBuffer&
  copyDataToImage(Image&                  dst,
                  std::span<const byte_t> src,
                  std::span<const VkBufferImageCopy2>) const;

  template <typename T>
  const CommandBuffer&
  copyBuffer(Buffer<T>&                     dst,
             const Buffer<T>&               src,
             std::span<const VkBufferCopy2> copy_regions) const
  {
    Assert(dst.allocSize() >= src.allocSize());
    const VkCopyBufferInfo2 copy_info{
      .sType       = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
      .pNext       = {},
      .srcBuffer   = src.vk(),
      .dstBuffer   = dst.vk(),
      .regionCount = static_cast<uint32_t>(copy_regions.size()),
      .pRegions    = copy_regions.data(),
    };
    return copyBuffer(copy_info);
  }

  template <typename T>
  const CommandBuffer&
  copyDataToBuffer(Buffer<T>&                     dst,
                   std::span<const T>             src,
                   std::span<const VkBufferCopy2> copy_regions) const
  {
    return copyBuffer(
      dst,
      createStagingBuffer(
        fmt::format("Staging buffer #{}", staging_buffers_.size() + 1),
        std::as_bytes(src)),
      copy_regions);
  }

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

  [[nodiscard]] const Buffer<byte_t>&
  createStagingBuffer(const std::string&      name,
                      std::span<const byte_t> data) const;

  [[nodiscard]] VkResult fenceStatus() const;
  void                   resetFence() const;
  void                   waitFence() const;

private:
  const CommandBuffer& copyBuffer(const VkCopyBufferInfo2& copy_info) const;

  // Specifically for staging buffer copies
  template <typename T>
  const CommandBuffer&
  copyBuffer(Buffer<T>&                     dst,
             const Buffer<byte_t>&          src,
             std::span<const VkBufferCopy2> copy_regions) const
  {
    Assert(dst.allocSize() >= src.allocSize());
    const VkCopyBufferInfo2 copy_info{
      .sType       = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
      .pNext       = {},
      .srcBuffer   = src.vk(),
      .dstBuffer   = dst.vk(),
      .regionCount = static_cast<uint32_t>(copy_regions.size()),
      .pRegions    = copy_regions.data(),
    };
    return copyBuffer(copy_info);
  }
  //----------------------------------------------------------------------------
  // Only for use in AllocatedBuffer
  friend AllocatedBuffer;
  const CommandBuffer& copyDataToBuffer(AllocatedBuffer&        dst,
                                        std::span<const byte_t> src) const;
  //----------------------------------------------------------------------------

private:
  const CommandPool*                  command_pool_{ nullptr };
  Fence                               wait_fence_;
  mutable std::vector<Buffer<byte_t>> staging_buffers_;
};
NAMESPACE_END(eldr::vk::wr)
