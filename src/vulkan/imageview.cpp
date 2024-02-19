#include <eldr/core/logger.hpp>
#include <eldr/vulkan/imageview.hpp>
#include <vulkan/vulkan_core.h>

namespace eldr {
namespace vk {
ImageView::ImageView()
  : device_(nullptr), image_view_(VK_NULL_HANDLE), format_(VK_FORMAT_UNDEFINED)
{
}
ImageView::ImageView(const Device* device, VkImage image, VkFormat format)
  : device_(device)
{
  VkImageViewCreateInfo image_view_ci{};
  image_view_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_ci.image    = image;
  image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_ci.format   = format;
  // Standard color properties
  image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  image_view_ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_ci.subresourceRange.baseMipLevel   = 0;
  image_view_ci.subresourceRange.levelCount     = 1;
  image_view_ci.subresourceRange.baseArrayLayer = 0;
  image_view_ci.subresourceRange.layerCount     = 1;

  if (vkCreateImageView(device_->logical(), &image_view_ci, nullptr,
                        &image_view_) != VK_SUCCESS) {
    ThrowVk("Failed to create image view!");
  }
}

ImageView::ImageView(ImageView&& other)
  : device_(other.device_), image_view_(other.image_view_),
    format_(other.format_)
{
  other.device_     = nullptr;
  other.image_view_ = VK_NULL_HANDLE;
  other.format_     = VK_FORMAT_UNDEFINED;
}

ImageView::~ImageView()
{
  if (image_view_ != VK_NULL_HANDLE)
    vkDestroyImageView(device_->logical(), image_view_, nullptr);
}

ImageView& ImageView::operator=(ImageView&& other)
{
  if (this != &other) {
    if (image_view_ != VK_NULL_HANDLE)
      vkDestroyImageView(device_->logical(), image_view_, nullptr);

    device_           = other.device_;
    image_view_       = other.image_view_;
    format_           = other.format_;
    other.device_     = nullptr;
    other.image_view_ = VK_NULL_HANDLE;
    other.format_     = VK_FORMAT_UNDEFINED;
  }
  return *this;
}
} // namespace vk
} // namespace eldr
