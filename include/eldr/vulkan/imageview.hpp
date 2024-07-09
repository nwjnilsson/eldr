#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/image.hpp>

namespace eldr {
namespace vk {

class ImageView {
public:
  ImageView();
  ImageView(const Device*, VkImage, VkFormat,
            VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
  ImageView(ImageView&&);
  ~ImageView();

  ImageView& operator=(ImageView&&);

  const VkImageView& get() const { return image_view_; }

private:
  const Device* device_;

  VkImageView image_view_;
  VkFormat    format_;
};
} // namespace vk
} // namespace eldr
