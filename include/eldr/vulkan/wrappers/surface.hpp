#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Surface : public VkObject<VkSurfaceKHR> {
  using Base = VkObject<VkSurfaceKHR>;

public:
  EL_VK_IMPORT_DEFAULTS(Surface)
  Surface(std::string_view name, const Instance&, const Window&);

private:
  const Instance* instance_{ nullptr };
};
NAMESPACE_END(eldr::vk::wr)
