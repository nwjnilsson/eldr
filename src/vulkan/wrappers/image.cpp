#include <eldr/core/bitmap.hpp>

#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

NAMESPACE_BEGIN()
uint32_t calculateMipLevels(const Bitmap& bitmap)
{
  return static_cast<uint32_t>(
           std::floor(std::log2(std::max(bitmap.width(), bitmap.height())))) +
         1;
}

ImageCreateInfo createBitmapTextureCI(const Bitmap& bitmap, uint32_t mip_levels)
{
  const VkExtent2D size{ bitmap.width(), bitmap.height() };
  // Calculate mip level count to generate mip levels

  VkFormat format;
  // Pixel format should be RGBA at this point
  Assert(bitmap.pixelFormat() == Bitmap::PixelFormat::RGBA);
  switch (bitmap.componentFormat()) {
    case StructType::UInt8:
      if (bitmap.srgbGamma())
        format = VK_FORMAT_R8G8B8A8_SRGB;
      else
        format = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    default:
      Throw("Component formats other than UInt8 are not yet implemented");
  }

  ImageCreateInfo texture_info{
    .name        = bitmap.name(),
    .extent      = size,
    .format      = format,
    .tiling      = VK_IMAGE_TILING_OPTIMAL,
    .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    .sample_count = VK_SAMPLE_COUNT_1_BIT,
    .mip_levels   = mip_levels,
    .memory_usage = VMA_MEMORY_USAGE_GPU_ONLY,
    .final_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  };
  return texture_info;
}
NAMESPACE_END()
//------------------------------------------------------------------------------
// Image
//------------------------------------------------------------------------------
Image::Image()                   = default;
Image::Image(Image&&) noexcept   = default;
Image& Image::operator=(Image&&) = default;

Image::Image(const Device& device, const ImageCreateInfo& image_info)
  : Base(image_info.name, device), size_(image_info.extent),
    format_(image_info.format), mip_levels_(image_info.mip_levels)
{
  const VkImageCreateInfo image_ci{
    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext                 = nullptr,
    .flags                 = 0,
    .imageType             = VK_IMAGE_TYPE_2D,
    .format                = format_,
    .extent                = { size_.width, size_.height, 1 },
    .mipLevels             = mip_levels_,
    .arrayLayers           = 1,
    .samples               = image_info.sample_count,
    .tiling                = image_info.tiling,
    .usage                 = image_info.usage_flags,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = {},
    .pQueueFamilyIndices   = nullptr,
    .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  const VmaAllocationCreateInfo alloc_ci{
    .flags          = {},
    .usage          = image_info.memory_usage,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };
  if (const VkResult result{ vmaCreateImage(device.allocator(),
                                            &image_ci,
                                            &alloc_ci,
                                            vkp(),
                                            &allocation_,
                                            &alloc_info_) };
      result != VK_SUCCESS)
    Throw("Failed to create image! ({})", result);

  image_view_ = ImageView{ device, *this, image_info.aspect_flags };

  if (image_info.final_layout != VK_IMAGE_LAYOUT_UNDEFINED) {
    device.execute([&](const CommandBuffer& cb) {
      cb.transitionImageLayout(*this, image_info.final_layout);
    });
  }
}

Image::Image(const Device& device, const Bitmap& bitmap, uint32_t mip_levels)
  : Image(device, createBitmapTextureCI(bitmap, mip_levels))
{
  //----------------------------------------------------------------------------
  // Copy copy bitmap data and generate mipmap
  //----------------------------------------------------------------------------
  const VkBufferImageCopy2 copy_regions[] { {
    .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
      .pNext = {},
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
  }};

  device.execute([&](const CommandBuffer& cb) {
    cb.copyDataToImage(*this, bitmap.bytes(), copy_regions)
      // The transition below is not necessary when generating mipmaps using the
      // blit command, since each level will be transitioned to
      // VK_IMAGE_LAYOUT_SHADER_READ_ONLY after the blit command is finished.
      //.transitionImageLayout(image_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      // TODO: real-time generation of mipmaps like this is not common practice
      // . Add support for loading each mip level from disk (making the
      // transition above necessary again (I think)).
      .generateMipmaps(*this);
  });
}

Image::Image(const Device& device, const Bitmap& bitmap)
  : Image(device, bitmap, calculateMipLevels(bitmap))
{
}

Image::~Image()
{
  // Swapchain images will not have an allocation, don't free those
  if (vk() and allocation_) {
    vmaDestroyImage(device().allocator(), vk(), allocation_);
  }
}

Image Image::createErrorImage(const Device& device)
{
  return Image{ device, Bitmap::createCheckerboard() };
}

Image Image::createSwapchainImage(const Device&    device,
                                  VkImage          vkimage,
                                  std::string_view name,
                                  VkExtent2D       extent,
                                  VkFormat         format)
{
  Image image;
  image.name_   = name;
  image.size_   = extent;
  image.format_ = format;
  image.vk()    = vkimage;
  const ImageViewCreateInfo image_view_ci{
    .image        = image.vk(),
    .format       = format,
    .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    .mip_levels   = image.mip_levels_,
  };
  image.image_view_ = ImageView{ device, image_view_ci };
  return image;
}

NAMESPACE_END(eldr::vk::wr)
