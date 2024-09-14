#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/commandpool.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/image.hpp>
#include <eldr/vulkan/imageview.hpp>

namespace eldr {
namespace vk {

class Texture {
public:
  Texture(const Device*, CommandPool&);

  const ImageView& view() const { return image_view_; }
  uint32_t mipLevels() const { return mip_levels_; }

  Texture& operator=(Texture&&);

private:
  Image     image_;
  ImageView image_view_;
  uint32_t  mip_levels_;
};
void generateMipmaps(const Device*, CommandPool&, const Image& image,
                     uint32_t mip_levels);
} // namespace vk
} // namespace eldr
