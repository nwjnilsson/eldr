#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// ImageViewImpl
//------------------------------------------------------------------------------
class ImageView::ImageViewImpl {
public:
  ImageViewImpl(const Device&                device,
                const VkImageViewCreateInfo& image_view_ci);
  ~ImageViewImpl();
  const Device& device_;
  VkImageView   image_view_{ VK_NULL_HANDLE };
};

ImageView::ImageViewImpl::ImageViewImpl(
  const Device& device, const VkImageViewCreateInfo& image_view_ci)
  : device_(device)
{
  if (const VkResult result = vkCreateImageView(
        device_.logical(), &image_view_ci, nullptr, &image_view_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateImageView(): ");
}

ImageView::ImageViewImpl::~ImageViewImpl()
{
  vkDestroyImageView(device_.logical(), image_view_, nullptr);
}

//------------------------------------------------------------------------------
// ImageViewImpl
//------------------------------------------------------------------------------
ImageView::ImageView(const Device& device, const GpuImage& image,
                     VkImageAspectFlags aspect_flags)
{
  const VkImageViewCreateInfo image_view_ci{
    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = {},
    .image    = image.get(),
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format   = image.format(),
    // Standard color properties
    .components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY },
    .subresourceRange = { 
      .aspectMask     = aspect_flags,
      .baseMipLevel   = 0,
      .levelCount     = image.mipLevels(),
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
  };
  iv_data_ = std::make_shared<ImageViewImpl>(device, image_view_ci);
}

VkImageView ImageView::get() const { return iv_data_->image_view_; }
} // namespace eldr::vk::wr
