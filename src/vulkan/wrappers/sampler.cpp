#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
//------------------------------------------------------------------------------
// SamplerImpl
//------------------------------------------------------------------------------
class Sampler::SamplerImpl {
public:
  SamplerImpl(const Device& device, const VkSamplerCreateInfo& ci);
  ~SamplerImpl();
  const Device& device_;
  VkSampler     sampler_{ VK_NULL_HANDLE };
};

Sampler::SamplerImpl::SamplerImpl(const Device&              device,
                                  const VkSamplerCreateInfo& ci)
  : device_(device)
{
  if (const VkResult result{
        vkCreateSampler(device.logical(), &ci, nullptr, &sampler_) };
      result != VK_SUCCESS) {
    Throw("Failed to create sampler! ({})", result);
  }
}

Sampler::SamplerImpl::~SamplerImpl()
{
  vkDestroySampler(device_.logical(), sampler_, nullptr);
}

//------------------------------------------------------------------------------
// Sampler
//------------------------------------------------------------------------------
Sampler::Sampler()                     = default;
Sampler::Sampler(Sampler&&) noexcept   = default;
Sampler::~Sampler()                    = default;
Sampler& Sampler::operator=(Sampler&&) = default;

Sampler::Sampler(const Device&       device,
                 VkFilter            mag_filter,
                 VkFilter            min_filter,
                 VkSamplerMipmapMode mipmap_mode,
                 uint32_t            mip_levels)
{
  VkPhysicalDeviceProperties props{};
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

  d_ = std::make_unique<SamplerImpl>(device, sampler_info);
}

VkSampler Sampler::vk() const { return d_->sampler_; }

NAMESPACE_END(eldr::vk::wr)
