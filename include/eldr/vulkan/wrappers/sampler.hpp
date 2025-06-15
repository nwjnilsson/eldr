#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Sampler : public VkDeviceObject<VkSampler> {
  using Base = VkDeviceObject<VkSampler>;

public:
  EL_VK_IMPORT_DEFAULTS(Sampler)
  Sampler(std::string_view name,
          Device const&,
          VkFilter            mag_filter,
          VkFilter            min_filter,
          VkSamplerMipmapMode mipmap_mode,
          uint32_t            mip_levels);
};

NAMESPACE_END(eldr::vk::wr)
