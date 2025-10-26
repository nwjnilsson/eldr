#pragma once
#include <vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk)
class DebugUtilsMessenger : public VkInstanceObject<VkDebugUtilsMessengerEXT> {
  using Base = VkInstanceObject<VkDebugUtilsMessengerEXT>;

public:
  EL_VK_IMPORT_DEFAULTS(DebugUtilsMessenger)
  DebugUtilsMessenger(std::string_view name, const Instance& instance);
};
NAMESPACE_END(eldr::vk)
