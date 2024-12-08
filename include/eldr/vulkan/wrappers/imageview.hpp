#pragma once
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {
class ImageView {
public:
  ImageView() = default;
  ImageView(const Device&, const Image&, VkImageAspectFlags);

  [[nodiscard]] VkImageView get() const;

  /// @brief This function is used internally in the Swapchain class to create
  /// image views tied to the VkSwapchainKHR's internal images.
  [[nodiscard]] static ImageView createSwapchainImageView(const Device& device,
                                                          VkImage       image,
                                                          VkFormat      format);

private:
  ImageView(const Device&, const VkImageViewCreateInfo& image_view_ci);

protected:
  class ImageViewImpl;
  std::shared_ptr<ImageViewImpl> iv_data_;
};
} // namespace eldr::vk::wr
