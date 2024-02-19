#pragma once

#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/common.hpp>

namespace eldr {
namespace vk {

class Sampler {
public:
  Sampler(const Device*);
  ~Sampler();

  const VkSampler& get() const { return sampler_; }


private:
  const Device* device_;

  VkSampler sampler_;
};
} // namespace vk
} // namespace eldr
