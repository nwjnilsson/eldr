#pragma once
#include <app/fwd.hpp>
#include <vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk)

class Surface : public VkInstanceObject<VkSurfaceKHR> {
  using Base = VkInstanceObject<VkSurfaceKHR>;

public:
  EL_VK_IMPORT_DEFAULTS(Surface)
  Surface(std::string_view name, const Instance&, const Window&);
};
NAMESPACE_END(eldr::vk)
