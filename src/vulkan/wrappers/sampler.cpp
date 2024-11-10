#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk::wr {

Sampler::Sampler(const Device& device, uint32_t mip_levels) : device_(device)
{
  VkPhysicalDeviceProperties props{};
  vkGetPhysicalDeviceProperties(device_.physical(), &props);

  const VkSamplerCreateInfo sampler_info{
    .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext                   = {},
    .flags                   = {},
    .magFilter               = VK_FILTER_LINEAR,
    .minFilter               = VK_FILTER_LINEAR,
    .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .mipLodBias              = 0.0f,
    .anisotropyEnable        = VK_TRUE,
    .maxAnisotropy           = props.limits.maxSamplerAnisotropy,
    .compareEnable           = VK_FALSE,
    .compareOp               = VK_COMPARE_OP_ALWAYS,
    .minLod                  = 0.0f,
    .maxLod                  = static_cast<float>(mip_levels),
    .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
  };

  if (const auto result =
        vkCreateSampler(device_.logical(), &sampler_info, nullptr, &sampler_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateSampler(): ");
  }
}

Sampler::~Sampler()
{
  if (sampler_ != VK_NULL_HANDLE)
    vkDestroySampler(device_.logical(), sampler_, nullptr);
}

} // namespace eldr::vk::wr
