#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

struct ImageViewCreateInfo {
  VkImage            image;
  VkFormat           format;
  VkImageAspectFlags aspect_flags;
  uint32_t           mip_levels;
};

class ImageView : public VkDeviceObject<VkImageView> {
  using Base = VkDeviceObject<VkImageView>;

public:
  EL_VK_IMPORT_DEFAULTS(ImageView)
  ImageView(std::string_view name,
            const Device&,
            const ImageViewCreateInfo& image_view_ci);
  ImageView(const Image&, VkImageAspectFlags);

  [[nodiscard]] VkImageAspectFlags aspectFlags() const { return aspect_flags_; }

  /// @brief This function is used internally in the Swapchain class to create
  /// image views tied to the VkSwapchainKHR's internal images.
  //[[nodiscard]] static ImageView
  // swapchainImageView(const Device& device, VkImage image, VkFormat format);

private:
  VkImageAspectFlags aspect_flags_{ VK_IMAGE_ASPECT_NONE };
};
NAMESPACE_END(eldr::vk::wr)
