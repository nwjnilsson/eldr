#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk::wr {

class GpuTexture {
public:
  /// Create a GpuTexture. If n_mip_levels = 0, mipmaps will be generated based
  /// on the width and height of the texture.
  GpuTexture(const Device&, const uint8_t* data, VkDeviceSize data_size,
             uint32_t texture_width, uint32_t texture_height,
             uint32_t n_channels, uint32_t n_mip_levels,
             const std::string& name);

  /// Create a GpuTexture given a Bitmap. Will generate mipmaps based on
  /// dimensions of the Bitmap.
  GpuTexture(const Device&, const Bitmap&);
  /// Move constructor
  GpuTexture(GpuTexture&&) noexcept;
  ~GpuTexture() = default;

  GpuTexture& operator=(const GpuTexture&) = delete;
  GpuTexture& operator=(GpuTexture&&)      = delete;

  [[nodiscard]] uint32_t           mipLevels() const { return mip_levels_; }
  [[nodiscard]] VkImageView        imageView() const;
  [[nodiscard]] VkSampler          sampler() const;
  [[nodiscard]] const std::string& name() const { return name_; }

private:
  std::string               name_{};
  uint32_t                  channels_{};
  uint32_t                  mip_levels_{};
  std::unique_ptr<GpuImage> image_{};
  std::unique_ptr<Sampler>  sampler_{};
};
} // namespace eldr::vk::wr
