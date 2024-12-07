#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>

namespace eldr::vk::wr {
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
  if (const auto result =
        vkCreateSampler(device.logical(), &ci, nullptr, &sampler_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateSampler(): ");
  }
}

Sampler::SamplerImpl::~SamplerImpl()
{
  if (sampler_ != VK_NULL_HANDLE)
    vkDestroySampler(device_.logical(), sampler_, nullptr);
}

//------------------------------------------------------------------------------
// Sampler
//------------------------------------------------------------------------------
Sampler::Sampler(const Device& device, VkFilter filter, uint32_t mip_levels)
{
  VkPhysicalDeviceProperties props{};
  vkGetPhysicalDeviceProperties(device.physical(), &props);

  const VkSamplerCreateInfo sampler_info{
    .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext                   = {},
    .flags                   = {},
    .magFilter               = filter,
    .minFilter               = filter,
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

  sampler_data_ = std::make_shared<SamplerImpl>(device, sampler_info);
}

VkSampler Sampler::get() const { return sampler_data_->sampler_; }

} // namespace eldr::vk::wr
