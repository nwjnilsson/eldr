#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
class DebugUtilsMessenger : public VkObject<VkDebugUtilsMessengerEXT> {
  using Base = VkObject<VkDebugUtilsMessengerEXT>;

public:
  EL_VK_IMPORT_DEFAULTS(DebugUtilsMessenger)
  DebugUtilsMessenger(std::string_view name, const Instance& instance);

private:
  const Instance* instance_{ nullptr };
};
NAMESPACE_END(eldr::vk::wr)
