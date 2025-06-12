#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Sampler {

public:
  Sampler();
  Sampler(const Device&,
          VkFilter            mag_filter,
          VkFilter            min_filter,
          VkSamplerMipmapMode mipmap_mode,
          uint32_t            mip_levels);
  Sampler(Sampler&&) noexcept;
  ~Sampler();

  Sampler& operator=(Sampler&&);

  [[nodiscard]] VkSampler vk() const;

private:
  class SamplerImpl;
  std::unique_ptr<SamplerImpl> d_;
};
NAMESPACE_END(eldr::vk::wr)
