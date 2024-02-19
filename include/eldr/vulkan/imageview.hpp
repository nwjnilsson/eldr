#pragma once

#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/image.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr {
namespace vk {

class ImageView {
public:
  ImageView();
  ImageView(const Device*, VkImage, VkFormat);
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
