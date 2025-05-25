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
  ImageImpl(const Device&                  device,
            const VkImageCreateInfo&       image_ci,
            const VmaAllocationCreateInfo& alloc_ci);
  ~ImageImpl();
  VkImage image_{ VK_NULL_HANDLE };
};

Image::ImageImpl::ImageImpl(const Device&                  device,
                            const VkImageCreateInfo&       image_ci,
                            const VmaAllocationCreateInfo& alloc_ci)
  : GpuResourceAllocation(device)
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
  d_          = std::make_shared<ImageImpl>(device, image_ci, alloc_ci);
  image_view_ = ImageView{ device, *this, image_info.aspect_flags };

  if (image_info.final_layout != VK_IMAGE_LAYOUT_UNDEFINED) {
    device.execute([&](const CommandBuffer& cb) {
      cb.transitionImageLayout(*this, image_info.final_layout);
    });
  }
}

VkImage Image::get() const { return d_->image_; }
} // namespace eldr::vk::wr
