#include <vulkan/wrappers/device.hpp>
#include <vulkan/wrappers/image.hpp>

NAMESPACE_BEGIN(eldr::vk)
NAMESPACE_BEGIN()
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

NAMESPACE_END()

ImageView ::ImageView()                     = default;
ImageView ::ImageView(ImageView&&) noexcept = default;
ImageView& ImageView ::operator=(ImageView&& o)
{
  if (this != &o) {
    if (vk()) {
      vkDestroyImageView(device().logical(), object_, nullptr);
    }
    aspect_flags_ = o.aspect_flags_;
    Base ::operator=(std ::move(o));
  }
  return *this;
}
ImageView ::~ImageView()
{
  if (vk()) {
    vkDestroyImageView(device().logical(), object_, nullptr);
  }
}

ImageView::ImageView(std::string_view           name,
                     const Device&              device,
                     const ImageViewCreateInfo& image_view_ci)
  : Base(name, device), aspect_flags_(image_view_ci.aspect_flags)
{
  const auto view_ci = getVkImageViewCI(image_view_ci);
  if (const VkResult result{
        vkCreateImageView(device.logical(), &view_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create image view! ({})", result);
}

ImageView::ImageView(const Image& image, VkImageAspectFlags aspect_flags)
  : Base(image.name(), image.device()), aspect_flags_(aspect_flags)
{
  const ImageViewCreateInfo image_view_ci{
    .image        = image.vk(),
    .format       = image.format(),
    .aspect_flags = aspect_flags,
    .mip_levels   = image.mipLevels(),
  };

  const auto view_ci = getVkImageViewCI(image_view_ci);
  if (const VkResult result{
        vkCreateImageView(device().logical(), &view_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create image view! ({})", result);
}

NAMESPACE_END(eldr::vk)
