#include <eldr/core/hash.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
Sampler::Sampler()                              = default;
Sampler::Sampler(Sampler&&) noexcept            = default;
Sampler& Sampler::operator=(Sampler&&) noexcept = default;

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
        vkCreateSampler(device.logical(), &sampler_info, nullptr, vkp()) };
      result != VK_SUCCESS) {
    Throw("Failed to create sampler! ({})", result);
  }
}

Sampler::~Sampler()
{
  if (vk()) {
    vkDestroySampler(device().logical(), vk(), nullptr);
  }
}

// size_t hash(Sampler const& s)
// {
//   size_t value{ eldr::hash<std::string>(s.name()) };
//   value = hashCombine(value, eldr::hash<VkDevice>(s.device_->logical()));
//   value = hashCombine(value, eldr::hash<VkFilter>(s.min_filter_));
//   value = hashCombine(value, eldr::hash<VkFilter>(s.mag_filter_));
//   value = hashCombine(value,
//   eldr::hash<VkSamplerMipmapMode>(s.mipmap_mode_)); value =
//   hashCombine(value, eldr::hash<uint32_t>(s.mip_levels_)); return value;
// };

// bool Sampler::operator==(Sampler const& o)
// {
//   return name() == o.name() and mag_filter_ == o.mag_filter_ and
//          min_filter_ == o.min_filter_ and mipmap_mode_ == o.mipmap_mode_ and
//          mip_levels_ == o.mip_levels_;
// }

NAMESPACE_END(eldr::vk::wr)
// size_t std::hash<eldr::vk::wr::Sampler>::operator()(
//   eldr::vk::wr::Sampler const& sampler) const
// {
//   return eldr::vk::wr::hash(sampler);
// };
