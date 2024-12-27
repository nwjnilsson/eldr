#include <eldr/core/fstream.hpp>
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/texture.hpp>

namespace eldr::vk::wr {
namespace {
Image::ImageCreateInfo
getTextureCreateInfo(const Bitmap&                  bitmap,
                     const std::optional<uint32_t>& mip_levels)
{
  const VkExtent2D size{ bitmap.width(), bitmap.height() };
  // Calculate mip level count to generate mip levels
  const uint32_t n_mip_levels{ mip_levels.has_value()
                                 ? mip_levels.value()
                                 : static_cast<uint32_t>(std::floor(std::log2(
                                     std::max(size.width, size.height)))) +
                                     1 };

  VkFormat format;
  // Pixel format should be RGBA at this point
  assert(bitmap.pixelFormat() == Bitmap::PixelFormat::RGBA);
  switch (bitmap.componentFormat()) {
    case Struct::Type::UInt8:
      if (bitmap.srgbGamma())
        format = VK_FORMAT_R8G8B8A8_SRGB;
      else
        format = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    default:
      Throw("getTextureCreateInfo(): component formats other than UInt8 are "
            "not yet implemented");
  }

  Image::ImageCreateInfo texture_info{
    .name        = bitmap.name(),
    .extent      = size,
    .format      = format,
    .tiling      = VK_IMAGE_TILING_OPTIMAL,
    .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    .sample_count = VK_SAMPLE_COUNT_1_BIT,
    .mip_levels   = n_mip_levels,
    .memory_usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };
  return texture_info;
}
} // namespace

Texture::Texture(const Device& device, const Bitmap& bitmap,
                 std::optional<uint32_t> mip_levels)
  : Image(device, getTextureCreateInfo(bitmap, mip_levels))
{
  //----------------------------------------------------------------------------
  // Copy data to image and transition layout
  //----------------------------------------------------------------------------
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
    .imageExtent = { .width  = size_.width,
                     .height = size_.height,
                     .depth  = 1 },
  };
  device.execute([&](const CommandBuffer& cb) {
    cb.transitionImageLayout(*this, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
      .copyDataToImage(bitmap.uint8Data(), bitmap.bufferSize(), *this,
                       copy_region)
      // The transition below is not necessary when generating mipmaps using the
      // blit command, since each level will be transitioned to
      // VK_IMAGE_LAYOUT_SHADER_READ_ONLY after the blit command is finished.
      //.transitionImageLayout(image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      //                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      // TODO: real-time generation of mipmaps like this is not common practice
      // . Add support for loading each mip level from disk (making the
      // transition above necessary again (I think)).
      .generateMipmaps(*this);
  });
}

} // namespace eldr::vk::wr
