#pragma once

#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {

class Sampler {
public:
  Sampler(const Device&, uint32_t mip_levels);
  ~Sampler();

  const VkSampler& get() const { return sampler_; }

private:
  const Device& device_;

  VkSampler sampler_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
