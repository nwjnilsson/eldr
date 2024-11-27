#include <eldr/vulkan/wrappers/fence.hpp>

namespace eldr::vk::wr {

Fence::Fence(const Device& device) : device_(device)
{
  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if (const auto result =
        vkCreateFence(device_.logical(), &fence_ci, nullptr, &fence_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateFence(): ");
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

void Fence::reset() const { vkResetFences(device_.logical(), 1, &fence_); }

VkResult Fence::wait(uint64_t timeout) const
{
  const VkResult result =
    vkWaitForFences(device_.logical(), 1, &fence_, VK_TRUE, timeout);
  switch (result) {
    case VK_SUCCESS:
    case VK_TIMEOUT:
      return result;
    default:
      ThrowVk(result, "vkWaitForFences(): ");
  }
}

VkResult Fence::status() const
{
  return vkGetFenceStatus(device_.logical(), fence_);
}
} // namespace eldr::vk::wr
