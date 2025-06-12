#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/semaphore.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
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
  if (const VkResult result{ vkCreateSemaphore(
        device_.logical(), &semaphore_ci, nullptr, &semaphore_) };
      result != VK_SUCCESS)
    Throw("Failed to create semaphore ({})", result);
}

Semaphore::SemaphoreImpl::~SemaphoreImpl()
{
  vkDestroySemaphore(device_.logical(), semaphore_, nullptr);
}

//------------------------------------------------------------------------------
// Semaphore
//------------------------------------------------------------------------------
Semaphore::Semaphore()                     = default;
Semaphore::Semaphore(Semaphore&&) noexcept = default;
Semaphore::~Semaphore()                    = default;

Semaphore::Semaphore(const Device& device, VkSemaphoreCreateFlags flags)
{
  const VkSemaphoreCreateInfo semaphore_ci{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = {},
    .flags = flags,
  };
  d_ = std::make_unique<SemaphoreImpl>(device, semaphore_ci);
}

VkSemaphore        Semaphore::vk() const { return d_->semaphore_; }
const VkSemaphore* Semaphore::vkp() const { return &d_->semaphore_; }

NAMESPACE_END(eldr::vk::wr)
