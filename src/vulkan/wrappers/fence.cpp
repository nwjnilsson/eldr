#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

namespace eldr::vk::wr {

Fence::Fence(Device& device) : device_(device)
{
  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  CheckVkResult(vkCreateFence(device_.logical(), &fence_ci, nullptr, &fence_));
}

Fence::Fence(Fence&& other) : device_(other.device_), fence_(other.fence_)
{
  other.fence_ = VK_NULL_HANDLE;
}

Fence::~Fence()
{
  if (fence_ != VK_NULL_HANDLE)
    vkDestroyFence(device_.logical(), fence_, nullptr);
}

void Fence::reset() { vkResetFences(device_.logical(), 1, &fence_); }

void Fence::wait()
{
  vkWaitForFences(device_.logical(), 1, &fence_, VK_TRUE, UINT64_MAX);
}
} // namespace eldr::vk::wr
