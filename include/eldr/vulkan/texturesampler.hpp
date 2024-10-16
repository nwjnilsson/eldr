#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/texture.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk {

struct TextureSampler {
  // TODO: figure out how to fix this
  Texture                      texture;
  std::unique_ptr<wr::Sampler> sampler;

  explicit TextureSampler(const wr::Device& device, const wr::CommandPool& command_pool)
    : texture(device, command_pool),
      sampler(std::make_unique<wr::Sampler>(device, texture.mipLevels()))
  {
  }
};
} // namespace eldr::vk
