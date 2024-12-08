#pragma once
#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk::wr {

class GpuTexture {
public:
  GpuTexture() = default;
  /// Create a GpuTexture. If n_mip_levels = 0, mipmaps will be generated based
  /// on the width and height of the texture.
  GpuTexture(const Device&, const uint8_t* data, VkDeviceSize data_size,
             uint32_t texture_width, uint32_t texture_height,
             uint32_t n_channels, VkFormat format, uint32_t n_mip_levels,
             std::string_view name);

  /// @brief Create a GpuTexture given a Bitmap.
  /// @detail Generates mipmaps based on the dimensions of the Bitmap.
  GpuTexture(const Device& device, const Bitmap& bitmap);

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] uint32_t           channels() const { return channels_; }
  [[nodiscard]] uint32_t           mipLevels() const { return mip_levels_; }
  [[nodiscard]] const ImageView&   imageView() const { return image_.view(); }
  [[nodiscard]] const Sampler&     sampler() const { return sampler_; }

private:
  std::string name_;
  uint32_t    channels_{ 1 };
  uint32_t    mip_levels_{ 1 };
  Image       image_;
  Sampler     sampler_;
};
} // namespace eldr::vk::wr
