#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Fence : public VkDeviceObject<VkFence> {
  using Base = VkDeviceObject<VkFence>;

public:
  EL_VK_IMPORT_DEFAULTS(Fence)
  Fence(std::string_view name, const Device& device);

  VkResult reset() const;
  VkResult wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
  [[nodiscard]] VkResult status() const;
};
NAMESPACE_END(eldr::vk::wr)
