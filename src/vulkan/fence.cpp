#include <eldr/core/logger.hpp>
#include <eldr/vulkan/fence.hpp>

namespace eldr {
namespace vk {
Fence::Fence() : device_(nullptr), fence_(VK_NULL_HANDLE) {}

Fence::Fence(const Device* device) : device_(device)
{
  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateFence(device_->logical(), &fence_ci, nullptr, &fence_) !=
      VK_SUCCESS)
    ThrowVk("Failed to create semaphore!");
}

Fence::Fence(Fence&& other)
{
  device_       = other.device_;
  fence_        = other.fence_;
  other.device_ = nullptr;
  other.fence_  = VK_NULL_HANDLE;
}

Fence::~Fence()
{
  if (fence_ != VK_NULL_HANDLE)
    vkDestroyFence(device_->logical(), fence_, nullptr);
}
} // namespace vk
} // namespace eldr
