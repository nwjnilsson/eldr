#pragma once
#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

#include <optional>

namespace eldr::vk::wr {

class Texture : public Image {
public:
  Texture() = default;

  /// @brief Constructs a `Texture`, copies the data from `bitmap` and
  /// transitions the layout to `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`.
  /// @detail If `mip_levels` is not provided, mipmaps are generated based on
  /// the dimensions of `bitmap`.
  Texture(const Device& device, const Bitmap& bitmap,
          std::optional<uint32_t> mip_levels = std::nullopt);
};
} // namespace eldr::vk::wr
