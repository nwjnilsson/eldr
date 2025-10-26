#include <vulkan/wrappers/device.hpp>
#include <vulkan/wrappers/semaphore.hpp>

NAMESPACE_BEGIN(eldr::vk)
EL_VK_IMPL_DEV_DEFAULTS(Semaphore)
Semaphore::Semaphore(std::string_view       name,
                     const Device&          device,
                     VkSemaphoreCreateFlags flags)
  : Base(name, device)
{
  const VkSemaphoreCreateInfo semaphore_ci{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = {},
    .flags = flags,
  };
  if (const VkResult result{
        vkCreateSemaphore(device.logical(), &semaphore_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create semaphore ({})", result);
}

NAMESPACE_END(eldr::vk)
