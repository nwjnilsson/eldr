#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/commandbuffer.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// GpuImageImpl
//------------------------------------------------------------------------------
class Image::ImageImpl : public GpuResourceAllocation {
public:
  ImageImpl(const Device& device, const VkImageCreateInfo& image_ci);
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
Image::Image(const Device& device, std::string_view name,
             const ImageInfo& image_info, VmaMemoryUsage memory_usage)
  : name_(name), format_(image_info.format), size_(image_info.extent)
{
  // ---------------------------------------------------------------------------
  // Create image view
  // ---------------------------------------------------------------------------
  VkImageCreateInfo image_ci{
    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext                 = nullptr,
    .flags                 = 0,
    .imageType             = VK_IMAGE_TYPE_2D,
    .format                = format_,
    .extent                = { size_.width, size_.height, 1 },
    .mipLevels             = image_info.mip_levels,
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
    .usage          = memory_usage,
    .requiredFlags  = {},
    .preferredFlags = {},
    .memoryTypeBits = {},
    .pool           = {},
    .pUserData      = {},
    .priority       = {},
  };
  i_data_ = std::make_shared<ImageImpl>(device, image_ci, alloc_ci);
}

} // namespace eldr::vk::wr
