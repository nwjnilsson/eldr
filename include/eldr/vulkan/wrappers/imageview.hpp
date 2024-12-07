#pragma once

#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {
class ImageView {
public:
  ImageView(const Device&, const Image&, VkImageAspectFlags);

  [[nodiscard]] VkImageView get() const;

protected:
  class ImageViewImpl;
  std::shared_ptr<ImageViewImpl> iv_data_;
};
} // namespace eldr::vk::wr
