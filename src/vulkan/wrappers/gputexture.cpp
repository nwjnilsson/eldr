#include <eldr/core/fstream.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>

namespace eldr::vk::wr {

GpuTexture::GpuTexture(const Device& device, const uint8_t* data,
                       VkDeviceSize data_size, uint32_t texture_width,
                       uint32_t texture_height, uint32_t n_channels,
                       VkFormat format, uint32_t n_mip_levels,
                       std::string_view name)
  : name_(name), channels_(n_channels)
{
  // Calculate mip level count to generate mip levels
  mip_levels_ = n_mip_levels == 0
                  ? static_cast<uint32_t>(std::floor(
                      std::log2(std::max(texture_width, texture_height)))) +
                      1
                  : n_mip_levels;
  // Create sampler
  sampler_ = Sampler{ device, VK_FILTER_LINEAR, mip_levels_ };

  // ---------------------------------------------------------------------------
  // Create image
  // ---------------------------------------------------------------------------
  ImageInfo image_info{ .extent      = { texture_width, texture_height },
                        .format      = format,
                        .tiling      = VK_IMAGE_TILING_OPTIMAL,
                        .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                        .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
                        .sample_count = VK_SAMPLE_COUNT_1_BIT,
                        .mip_levels   = mip_levels_ };

  const VmaAllocationCreateInfo alloc_ci{
    .flags          = {},
    .usage          = VMA_MEMORY_USAGE_GPU_ONLY,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };

  image_ = GpuImage{ device, image_info, alloc_ci, name_ };

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
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels_)
      .copyBufferToImage(data, data_size, image_->get(), copy_region, name_)
      // The transition below is not necessary when generating mipmaps using the
      // blit command, since each level will be transitioned to
      // VK_IMAGE_LAYOUT_SHADER_READ_ONLY after the blit command is finished.
      //.transitionImageLayout(*image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      //                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      // TODO: real-time generation of mipmaps like this is not common practice.
      // Add support for loading each mip level from disk (making the transition
      // above necessary again (I think)).
      .generateMipmaps(*image_, mip_levels_);
  });
}

GpuTexture::GpuTexture(const Device& device, const Bitmap& bitmap)
  : GpuTexture(device, bitmap.uint8Data(),
               static_cast<VkDeviceSize>(bitmap.bufferSize()), bitmap.width(),
               bitmap.height(), bitmap.channelCount(), VK_FORMAT_R8G8B8A8_SRGB,
               0, bitmap.name())
{
}

const GpuImage& GpuTexture::image() const { return *image_; }

const Sampler& GpuTexture::sampler() const { return *sampler_; }

} // namespace eldr::vk::wr
