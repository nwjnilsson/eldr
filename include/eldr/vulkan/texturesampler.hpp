#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/texture.hpp>
#include <eldr/vulkan/sampler.hpp>

namespace eldr {
namespace vk {

struct TextureSampler {
  // TODO: figure out how to fix this. The sampler and texture/image should
  // probably be a combined object
  Texture texture;
  Sampler sampler;

  TextureSampler(const Device* device, CommandPool& command_pool)
    : texture(device, command_pool), sampler(device)
  {
  }
};
} // namespace vk
} // namespace eldr
