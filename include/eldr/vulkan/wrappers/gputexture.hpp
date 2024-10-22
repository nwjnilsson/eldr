#pragma once

#include <eldr/core/bitmap.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {

class GpuTexture {
public:
  GpuTexture(const Device&, const Bitmap&, const std::string& name);

  uint32_t mipLevels() const;

  GpuTexture& operator=(GpuTexture&&);

private:
  std::string               name_;
  std::unique_ptr<GpuImage> image_;
  std::unique_ptr<Sampler>  sampler_;
};
} // namespace eldr::vk::wr
