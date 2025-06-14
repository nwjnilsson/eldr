#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Sampler : public VkDeviceObject<VkSampler> {
  using Base = VkDeviceObject<VkSampler>;

public:
  Sampler();
  Sampler(std::string_view name,
          Device const&,
          VkFilter            mag_filter,
          VkFilter            min_filter,
          VkSamplerMipmapMode mipmap_mode,
          uint32_t            mip_levels);
  Sampler(Sampler&&) noexcept;
  ~Sampler();

  Sampler& operator=(Sampler&&) noexcept;

  // bool operator==(Sampler const& o);

  // friend size_t hash(Sampler const&);
  // private:
  //   VkFilter            mag_filter_;
  //   VkFilter            min_filter_;
  //   VkSamplerMipmapMode mipmap_mode_;
  //   uint32_t            mip_levels_;
};

NAMESPACE_END(eldr::vk::wr)

template <> struct std::hash<eldr::vk::wr::Sampler> {
  size_t operator()(eldr::vk::wr::Sampler const& sampler) const;
};
