#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr::vk {

class Texture {
public:
  Texture(const wr::Device&, const wr::CommandPool&);

  uint32_t mipLevels() const { return mip_levels_; }

  Texture& operator=(Texture&&);

private:
  std::unique_ptr<wr::Image> image_;
  uint32_t                   mip_levels_;
};
void generateMipmaps(const wr::Device&, wr::CommandPool&, wr::Image* image,
                     uint32_t mip_levels);
} // namespace eldr::vk
