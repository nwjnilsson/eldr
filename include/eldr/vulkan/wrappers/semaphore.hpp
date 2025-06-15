#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Semaphore : public VkDeviceObject<VkSemaphore> {
  using Base = VkDeviceObject<VkSemaphore>;

public:
  EL_VK_IMPORT_DEFAULTS(Semaphore)
  Semaphore(std::string_view       name,
            const Device&          device,
            VkSemaphoreCreateFlags flags = 0);
};
NAMESPACE_END(eldr::vk::wr)
