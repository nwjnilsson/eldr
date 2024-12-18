#include <eldr/core/fstream.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/texture.hpp>

namespace eldr::vk::wr {

Texture::Texture(const Device& device, std::string_view name,
                 const uint8_t* data, VkDeviceSize data_size, VkExtent2D extent,
                 uint32_t n_channels, VkFormat format,
                 std::optional<uint32_t> mip_levels)
  : name_(name), channels_(n_channels)
{
  // Calculate mip level count to generate mip levels
  const uint32_t n_mip_levels{ mip_levels.has_value()
                                 ? static_cast<uint32_t>(std::floor(std::log2(
                                     std::max(extent.width, extent.height)))) +
                                     1
                                 : mip_levels.value() };
  // ---------------------------------------------------------------------------
  // Create image
  // ---------------------------------------------------------------------------
  Image::ImageCreateInfo image_info{
    .name        = name_,
    .extent      = extent,
    .format      = format,
    .tiling      = VK_IMAGE_TILING_OPTIMAL,
    .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    .sample_count = VK_SAMPLE_COUNT_1_BIT,
    .mip_levels   = n_mip_levels,
  };

  image_ = Image{ device, image_info };

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
    .imageExtent = { .width  = image_.size().width,
                     .height = image_.size().height,
                     .depth  = 1 },
  };
  device.execute([&](const CommandBuffer& cb) {
    cb.transitionImageLayout(image_, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
      .copyBufferToImage(data, data_size, image_, copy_region)
      // The transition below is not necessary when generating mipmaps using the
      // blit command, since each level will be transitioned to
      // VK_IMAGE_LAYOUT_SHADER_READ_ONLY after the blit command is finished.
      //.transitionImageLayout(image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      //                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      // TODO: real-time generation of mipmaps like this is not common practice.
      // Add support for loading each mip level from disk (making the transition
      // above necessary again (I think)).
      .generateMipmaps(image_);
  });

  // Create sampler
  sampler_ = Sampler{ device, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                      VK_SAMPLER_MIPMAP_MODE_LINEAR, image_.mipLevels() };
}

// Texture::Texture(const Device& device, const Bitmap& bitmap,
//                        std::optional<uint32_t> mip_levels)
//   : name_(bitmap.name()), channels_(bitmap.channelCount()),
//     image_(device, bitmap, mip_levels)
// {
//   sampler_ = Sampler{ device, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
//                       VK_SAMPLER_MIPMAP_MODE_LINEAR, image_.mipLevels() };
// }
//

Texture::Texture(const Device& device, const Bitmap& bitmap,
                 std::optional<uint32_t> mip_levels)
  : Texture(device, bitmap.name(), bitmap.uint8Data(), bitmap.bufferSize(),
            VkExtent2D{ bitmap.width(), bitmap.height() },
            bitmap.channelCount(), VK_FORMAT_R8G8B8A8_SRGB, mip_levels)
{
}

} // namespace eldr::vk::wr
