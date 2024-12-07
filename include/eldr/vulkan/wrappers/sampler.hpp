#pragma once

#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {

class Sampler {

public:
  Sampler(const Device&, VkFilter filter, uint32_t mip_levels);

  [[nodiscard]] VkSampler get() const;

private:
  class SamplerImpl;
  std::shared_ptr<SamplerImpl> sampler_data_;
};
} // namespace eldr::vk::wr
