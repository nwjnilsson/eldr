#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/image.hpp>
#include <eldr/vulkan/imageview.hpp>
#include <eldr/vulkan/commandpool.hpp>

namespace eldr {
namespace vk {

class Texture {
public:
  Texture(const Device*, CommandPool&);

  const ImageView& view() const { return image_view_; }

  Texture& operator=(Texture&&);


private:
  Image image_;
  ImageView image_view_;

};
} // namespace vk
} // namespace eldr
