#include <eldr/core/fstream.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/helpers.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk::wr {

GpuTexture::GpuTexture(const Device& device, const Bitmap& bitmap,
                       const std::string& name)
  : name_(name)
{
  // ---------------------------------------------------------------------------
  // Create image
  // ---------------------------------------------------------------------------
  uint32_t mip_levels =
    static_cast<uint32_t>(
      std::floor(std::log2(std::max(bitmap.width(), bitmap.height())))) +
    1;

  VkDeviceSize image_size =
    bitmap.width() * bitmap.height() * bitmap.channelCount();

  ImageInfo image_info{ .extent      = { bitmap.width(), bitmap.height() },
                        .format      = VK_FORMAT_R8G8B8A8_SRGB,
                        .tiling      = VK_IMAGE_TILING_OPTIMAL,
                        .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                        .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                        .num_samples  = VK_SAMPLE_COUNT_1_BIT,
                        .mip_levels   = mip_levels };

  const VmaAllocationCreateInfo alloc_ci{
    .flags          = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    .usage          = VMA_MEMORY_USAGE_GPU_ONLY,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };

  image_ = std::make_unique<GpuImage>(device, image_info, alloc_ci, name);

  // ---------------------------------------------------------------------------
  // Copy data to image and transition layout
  // ---------------------------------------------------------------------------
  const VkBufferImageCopy copy_region{
    .bufferOffset      = 0,
    .bufferRowLength   = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
      .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .mipLevel       = 0,
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
    .imageOffset = { 0, 0, 0 },
    .imageExtent = { .width  = image_->size().width,
                     .height = image_->size().height,
                     .depth  = 1 },
  };
  device.execute([&](const CommandBuffer& cb) {
    cb.transitionImageLayout(*image_, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
      .copyBufferToImage(bitmap.data(), image_size, image_->get(), copy_region,
                         name_)
      .generateMipmaps(*image_);
  });
  // image_->transitionLayout(command_pool, VK_IMAGE_LAYOUT_UNDEFINED,
  //                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  // image_->copyFromBuffer(staging_buffer, command_pool);

  //  image_.transitionLayout(command_pool,
  //  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  //                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //                          mip_levels_);

  // TODO: real-time generation of mipmaps like this is not common practice.
  // Add support for loading each mip level from disk

  // Create sampler TODO
  sampler_ = std::make_unique<Sampler>(device, mip_levels);
}

} // namespace eldr::vk::wr
