#include <eldr/vulkan/semaphore.hpp>
#include <eldr/core/logger.hpp>

namespace eldr {
namespace vk {
Semaphore::Semaphore() : device_(nullptr), semaphore_(VK_NULL_HANDLE) {}
Semaphore::Semaphore(const Device* device) : device_(device)
{
  VkSemaphoreCreateInfo semaphore_ci{};
  semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if (vkCreateSemaphore(device_->logical(), &semaphore_ci, nullptr,
                        &semaphore_) != VK_SUCCESS)
    ThrowVk("Failed to create semaphore!");
}

Semaphore::Semaphore(Semaphore&& other) {
  device_ = other.device_;
  semaphore_ = other.semaphore_;
  other.device_ = nullptr;
  other.semaphore_ = VK_NULL_HANDLE;
}
Semaphore::~Semaphore() {
  if (semaphore_ != VK_NULL_HANDLE)
    vkDestroySemaphore(device_->logical(), semaphore_, nullptr);
}
} // namespace vk
} // namespace eldr
