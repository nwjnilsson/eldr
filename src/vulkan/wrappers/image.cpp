#include <eldr/core/bitmap.hpp>

#include <eldr/vulkan/vktypes.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {
using Bitmap = core::Bitmap;

namespace {
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
    case core::StructType::UInt8:
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
} // namespace
//------------------------------------------------------------------------------
// GpuImageImpl
//------------------------------------------------------------------------------

class Image::ImageImpl : public GpuResourceAllocation {
public:
  ImageImpl(const Device&                  device,
            const VkImageCreateInfo&       image_ci,
            const VmaAllocationCreateInfo& alloc_ci);
  ImageImpl(const Device&, VkImage); // for swapchain images
  ~ImageImpl();
  VkImage image_{ VK_NULL_HANDLE };
};

Image::ImageImpl::ImageImpl(const Device&                  device,
                            const VkImageCreateInfo&       image_ci,
                            const VmaAllocationCreateInfo& alloc_ci)
  : GpuResourceAllocation(device, {}, {}, {})
{
  if (const VkResult result{ vmaCreateImage(device_.allocator(),
                                            &image_ci,
                                            &alloc_ci,
                                            &image_,
                                            &allocation_,
                                            &alloc_info_) };
      result != VK_SUCCESS)
    Throw("Failed to create image! ({})", result);
}

Image::ImageImpl::ImageImpl(const Device& device, VkImage image)
  : GpuResourceAllocation(device, {}, {}, {}), image_(image)
{
}

Image::ImageImpl::~ImageImpl()
{
  if (allocation_ != VK_NULL_HANDLE) {
    vmaDestroyImage(device_.allocator(), image_, allocation_);
  }
}

//------------------------------------------------------------------------------
// Image
//------------------------------------------------------------------------------
Image::Image()                   = default;
Image::Image(Image&&) noexcept   = default;
Image::~Image()                  = default;
Image& Image::operator=(Image&&) = default;

Image::Image(const Device& device, const ImageCreateInfo& image_info)
  : name_(image_info.name), size_(image_info.extent),
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
  d_          = std::make_unique<ImageImpl>(device, image_ci, alloc_ci);
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
  image.d_      = std::make_unique<ImageImpl>(device, vkimage);
  const ImageViewCreateInfo image_view_ci{
    .image        = image.d_->image_,
    .format       = format,
    .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    .mip_levels   = image.mip_levels_,
  };
  image.image_view_ = ImageView{ device, image_view_ci };
  return image;
}

VkImage Image::vk() const { return d_ ? d_->image_ : VK_NULL_HANDLE; }

} // namespace eldr::vk::wr
