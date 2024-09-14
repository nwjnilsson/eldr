#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>

namespace eldr::vk::wr {
Semaphore::Semaphore(Device& device) : device_(device)
{
  auto semaphore_ci{ makeInfo<VkSemaphoreCreateInfo>() };

  CheckVkResult(
    vkCreateSemaphore(device_.logical(), &semaphore_ci, nullptr, &semaphore_));
}

Semaphore::Semaphore(Semaphore&& other)
  : device_(other.device_), semaphore_(other.semaphore_)
{
  other.semaphore_ = VK_NULL_HANDLE;
}

Semaphore::~Semaphore()
{
  if (semaphore_ != VK_NULL_HANDLE)
    vkDestroySemaphore(device_.logical(), semaphore_, nullptr);
}
} // namespace eldr::vk::wr
