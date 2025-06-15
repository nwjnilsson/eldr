#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk::wr)

class Instance : public VkObject<VkInstance> {
  using Base = VkObject<VkInstance>;

public:
  EL_VK_IMPORT_DEFAULTS(Instance)
  Instance(std::string_view name,
           const VkApplicationInfo&,
           std::vector<const char*>&& extensions);
};

//[[nodiscard]] static bool isExtensionAvailable(const std::string& extension);
//[[nodiscard]] static bool isLayerSupported(const std::string& layer);
NAMESPACE_END(eldr::vk::wr)
