#pragma once
#include <eldr/vulkan/vulkan.hpp>

namespace eldr::vk::wr {

class Sampler {

public:
  Sampler() = default;
  Sampler(const Device&,
          VkFilter            mag_filter,
          VkFilter            min_filter,
          VkSamplerMipmapMode mipmap_mode,
          uint32_t            mip_levels);

  [[nodiscard]] VkSampler vk() const;

private:
  class SamplerImpl;
  std::shared_ptr<SamplerImpl> d_;
};
} // namespace eldr::vk::wr
