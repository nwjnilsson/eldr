#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// SemaphoreImpl
//------------------------------------------------------------------------------
class Semaphore::SemaphoreImpl {
public:
  SemaphoreImpl(const Device&                device,
                const VkSemaphoreCreateInfo& semaphore_ci);
  ~SemaphoreImpl();
  const Device& device_;
  VkSemaphore   semaphore_{ VK_NULL_HANDLE };
};

Semaphore::SemaphoreImpl::SemaphoreImpl(
  const Device& device, const VkSemaphoreCreateInfo& semaphore_ci)
  : device_(device)
{
  if (const auto result = vkCreateSemaphore(device_.logical(), &semaphore_ci,
                                            nullptr, &semaphore_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateSemaphore(): ");
}

Semaphore::SemaphoreImpl::~SemaphoreImpl()
{
  vkDestroySemaphore(device_.logical(), semaphore_, nullptr);
}

//------------------------------------------------------------------------------
// Semaphore
//------------------------------------------------------------------------------
Semaphore::Semaphore(const Device& device, VkSemaphoreCreateFlags flags)
{
  const VkSemaphoreCreateInfo semaphore_ci{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = {},
    .flags = flags,
  };
  s_data_ = std::make_shared<SemaphoreImpl>(device, semaphore_ci);
}

VkSemaphore        Semaphore::get() const { return s_data_->semaphore_; }
const VkSemaphore* Semaphore::ptr() const { return &s_data_->semaphore_; }
} // namespace eldr::vk::wr
