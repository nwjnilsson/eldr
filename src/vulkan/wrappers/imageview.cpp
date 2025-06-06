#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/image.hpp>

namespace eldr::vk::wr {
namespace {
VkImageViewCreateInfo getVkImageViewCI(const ImageViewCreateInfo& ci)
{
  const VkImageViewCreateInfo image_view_ci{
    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = nullptr,
    .flags = {},
    .image    = ci.image,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format   = ci.format,
    // Standard color properties
    .components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY },
    .subresourceRange = {
      .aspectMask     = ci.aspect_flags,
      .baseMipLevel   = 0,
      .levelCount     = ci.mip_levels,
      .baseArrayLayer = 0,
      .layerCount     = 1,
    },
  };
  return image_view_ci;
}

} // namespace
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
  if (const VkResult result{ vkCreateImageView(
        device_.logical(), &image_view_ci, nullptr, &image_view_) };
      result != VK_SUCCESS)
    Throw("Failed to create image view! ({})", result);
}

ImageView::ImageViewImpl::~ImageViewImpl()
{
  vkDestroyImageView(device_.logical(), image_view_, nullptr);
}

//------------------------------------------------------------------------------
// ImageView
//------------------------------------------------------------------------------
ImageView::ImageView()                       = default;
ImageView::ImageView(ImageView&&) noexcept   = default;
ImageView::~ImageView()                      = default;
ImageView& ImageView::operator=(ImageView&&) = default;

ImageView::ImageView(const Device&              device,
                     const ImageViewCreateInfo& image_view_ci)
  : aspect_flags_(image_view_ci.aspect_flags),
    d_(std::make_unique<ImageViewImpl>(device, getVkImageViewCI(image_view_ci)))
{
}

ImageView::ImageView(const Device&      device,
                     const Image&       image,
                     VkImageAspectFlags aspect_flags)
  : aspect_flags_(aspect_flags)
{
  const ImageViewCreateInfo image_view_ci{
    .image        = image.vk(),
    .format       = image.format(),
    .aspect_flags = aspect_flags,
    .mip_levels   = image.mipLevels(),
  };
  d_ = std::make_unique<ImageViewImpl>(device, getVkImageViewCI(image_view_ci));
}

VkImageView ImageView::vk() const { return d_->image_view_; }
} // namespace eldr::vk::wr
