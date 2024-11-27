#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>

namespace eldr::vk::wr {
Semaphore::Semaphore(const Device& device) : device_(device)
{
  const VkSemaphoreCreateInfo semaphore_ci{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = {},
    .flags = {},
  };

  if (const auto result = vkCreateSemaphore(device_.logical(), &semaphore_ci,
                                            nullptr, &semaphore_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateSemaphore(): ");
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
