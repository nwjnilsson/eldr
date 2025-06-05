#pragma once
#include <eldr/vulkan/vulkan.hpp>

namespace eldr::vk::wr {

struct ImageViewCreateInfo {
  VkImage            image;
  VkFormat           format;
  VkImageAspectFlags aspect_flags;
  uint32_t           mip_levels;
};

class ImageView {
public:
  ImageView();
  ImageView(const Device&, const ImageViewCreateInfo& image_view_ci);
  ImageView(const Device&, const Image&, VkImageAspectFlags);
  ImageView(ImageView&&) noexcept;
  ~ImageView();

  ImageView& operator=(ImageView&&);

  [[nodiscard]] VkImageAspectFlags aspectFlags() const { return aspect_flags_; }
  [[nodiscard]] VkImageView        vk() const;

  /// @brief This function is used internally in the Swapchain class to create
  /// image views tied to the VkSwapchainKHR's internal images.
  //[[nodiscard]] static ImageView
  // swapchainImageView(const Device& device, VkImage image, VkFormat format);

private:
  VkImageAspectFlags aspect_flags_{ VK_IMAGE_ASPECT_NONE };

  class ImageViewImpl;
  std::unique_ptr<ImageViewImpl> d_;
};
} // namespace eldr::vk::wr
