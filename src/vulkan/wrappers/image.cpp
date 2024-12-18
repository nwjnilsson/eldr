#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// GpuImageImpl
//------------------------------------------------------------------------------
class Image::ImageImpl : public GpuResourceAllocation {
public:
  ImageImpl(const Device& device, const VkImageCreateInfo& image_ci,
            const VmaAllocationCreateInfo& alloc_ci);
  ~ImageImpl();
  VkImage image_{ VK_NULL_HANDLE };
};

Image::ImageImpl::ImageImpl(const Device&                  device,
                            const VkImageCreateInfo&       image_ci,
                            const VmaAllocationCreateInfo& alloc_ci)
  : GpuResourceAllocation(device)
{
  if (const VkResult result =
        vmaCreateImage(device_.allocator(), &image_ci, &alloc_ci, &image_,
                       &allocation_, &alloc_info_);
      result != VK_SUCCESS)
    ThrowVk(result, "vmaCreateImage(): ");
}

Image::ImageImpl::~ImageImpl()
{
  vmaDestroyImage(device_.allocator(), image_, allocation_);
}

//------------------------------------------------------------------------------
// Image
//------------------------------------------------------------------------------
Image::Image(const Device& device, const ImageCreateInfo& image_info)
  : name_(image_info.name), size_(image_info.extent),
    format_(image_info.format), mip_levels_(image_info.mip_levels)
{
  VkImageCreateInfo image_ci{
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
  i_data_ = std::make_shared<ImageImpl>(device, image_ci, alloc_ci);

  image_view_ = ImageView{ device, *this, image_info.aspect_flags };
}

Image Image::createTextureImage(const Device& device, const Bitmap& bitmap,
                                std::optional<uint32_t> mip_levels)
{
  const VkExtent2D size{ bitmap.width(), bitmap.height() };
  // Calculate mip level count to generate mip levels
  const uint32_t n_mip_levels{ mip_levels.has_value()
                                 ? mip_levels.value()
                                 : static_cast<uint32_t>(std::floor(std::log2(
                                     std::max(size.width, size.height)))) +
                                     1 };

  Image::ImageCreateInfo image_info{
    .name        = bitmap.name(),
    .extent      = size,
    .format      = VK_FORMAT_R8G8B8A8_SRGB, // TODO: get from component format?
    .tiling      = VK_IMAGE_TILING_OPTIMAL,
    .usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    .sample_count = VK_SAMPLE_COUNT_1_BIT,
    .mip_levels   = n_mip_levels,
    .memory_usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  Image texture{ device, image_info };

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
    .imageExtent = { .width  = size.width,
                     .height = size.height,
                     .depth  = 1 },
  };
  device.execute([&](const CommandBuffer& cb) {
    cb.transitionImageLayout(texture, VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
      .copyBufferToImage(bitmap.uint8Data(), bitmap.bufferSize(), texture,
                         copy_region)
      // The transition below is not necessary when generating mipmaps using the
      // blit command, since each level will be transitioned to
      // VK_IMAGE_LAYOUT_SHADER_READ_ONLY after the blit command is finished.
      //.transitionImageLayout(image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      //                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
      // TODO: real-time generation of mipmaps like this is not common practice
      // . Add support for loading each mip level from disk (making the
      // transition above necessary again (I think)).
      .generateMipmaps(texture);
  });

  return texture;
}

VkImage Image::get() const { return i_data_->image_; }
} // namespace eldr::vk::wr
