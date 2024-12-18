#pragma once
#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

#include <optional>

namespace eldr::vk::wr {

class Texture {
public:
  Texture() = default;
  /// Create a Texture. If mip_levels is given a value, mipmaps will be
  /// generated based on the width and height of the texture.
  Texture(const Device&, std::string_view name, const uint8_t* data,
          VkDeviceSize data_size, VkExtent2D extent, uint32_t channels,
          VkFormat format, std::optional<uint32_t> mip_levels = std::nullopt);

  /// @brief Create a Texture given a Bitmap.
  /// @detail Generates mipmaps based on the dimensions of the Bitmap.
  Texture(const Device& device, const Bitmap& bitmap,
          std::optional<uint32_t> mip_levels = std::nullopt);

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] uint32_t           channels() const { return channels_; }
  [[nodiscard]] uint32_t mipLevels() const { return image_.mipLevels(); }
  [[nodiscard]] const ImageView& imageView() const { return image_.view(); }
  [[nodiscard]] const Sampler&   sampler() const { return sampler_; }

private:
  std::string name_;
  uint32_t    channels_{ 1 };
  Image       image_;
  Sampler     sampler_;
};
} // namespace eldr::vk::wr
