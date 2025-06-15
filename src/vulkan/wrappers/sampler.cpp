#include <eldr/core/hash.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(Sampler)
EL_VK_IMPL_DESTRUCTOR(Sampler)

Sampler::Sampler(std::string_view    name,
                 const Device&       device,
                 VkFilter            mag_filter,
                 VkFilter            min_filter,
                 VkSamplerMipmapMode mipmap_mode,
                 uint32_t            mip_levels)
  : Base(name, device)
{
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(device.physical(), &props);

  const VkSamplerCreateInfo sampler_info{
    .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext                   = {},
    .flags                   = {},
    .magFilter               = mag_filter,
    .minFilter               = min_filter,
    .mipmapMode              = mipmap_mode,
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

  if (const VkResult result{
        vkCreateSampler(device.logical(), &sampler_info, nullptr, &object_) };
      result != VK_SUCCESS) {
    Throw("Failed to create sampler! ({})", result);
  }
}
NAMESPACE_END(eldr::vk::wr)
