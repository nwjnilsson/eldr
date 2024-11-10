#include <eldr/core/fstream.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/gputexture.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk::wr {

GpuTexture::GpuTexture(const Device& device, const uint8_t* data,
                       VkDeviceSize data_size, uint32_t texture_width,
                       uint32_t texture_height, uint32_t n_channels,
                       uint32_t n_mip_levels, const std::string& name)
  : name_(name), channels_(n_channels)
{
  // ---------------------------------------------------------------------------
  // Create image
  // ---------------------------------------------------------------------------
  // Calculate mip levels for generating them (below)
  mip_levels_ = n_mip_levels == 0
                  ? static_cast<uint32_t>(std::floor(
                      std::log2(std::max(texture_width, texture_height)))) +
                      1
                  : n_mip_levels;

  ImageInfo image_info{ .extent      = { texture_width, texture_height },
                        .format      = VK_FORMAT_R8G8B8A8_SRGB,
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

  image_ = std::make_unique<GpuImage>(device, image_info, alloc_ci, name_);

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
  sampler_ = std::make_unique<Sampler>(device, mip_levels_);
}

GpuTexture::GpuTexture(const Device& device, const Bitmap& bitmap)
  : GpuTexture(device, bitmap.uint8Data(),
               static_cast<VkDeviceSize>(bitmap.bufferSize()), bitmap.width(),
               bitmap.height(), bitmap.channelCount(), 0, bitmap.name())
{
}

GpuTexture::GpuTexture(GpuTexture&& other) noexcept
{
  name_     = std::exchange(other.name_, "");
  channels_ = std::exchange(other.channels_, 0);
  image_    = std::move(other.image_);
  sampler_  = std::move(other.sampler_);
}

VkImageView GpuTexture::imageView() const { return image_->view(); }

VkSampler GpuTexture::sampler() const { return sampler_->get(); }

} // namespace eldr::vk::wr
