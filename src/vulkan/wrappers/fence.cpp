#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

//------------------------------------------------------------------------------
// FenceImpl
//------------------------------------------------------------------------------
class Fence::FenceImpl {
public:
  FenceImpl(const Device& device, const VkFenceCreateInfo& fence_ci);
  ~FenceImpl();
  const Device& device_;
  VkFence       fence_{ VK_NULL_HANDLE };
};

Fence::FenceImpl::FenceImpl(const Device&            device,
                            const VkFenceCreateInfo& fence_ci)
  : device_(device)
{
  if (const VkResult result{
        vkCreateFence(device_.logical(), &fence_ci, nullptr, &fence_) };
      result != VK_SUCCESS)
    Throw("Failed to create fence! ({})", result);
}

Fence::FenceImpl::~FenceImpl()
{
  vkDestroyFence(device_.logical(), fence_, nullptr);
}

//------------------------------------------------------------------------------
// Fence
//------------------------------------------------------------------------------
Fence::Fence()  = default;
Fence::~Fence() = default;
Fence::Fence(const Device& device)
{
  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  d_             = std::make_unique<FenceImpl>(device, fence_ci);
}

VkFence Fence::vk() const { return d_->fence_; }

VkResult Fence::reset() const
{
  return vkResetFences(d_->device_.logical(), 1, &d_->fence_);
}

VkResult Fence::wait(uint64_t timeout) const
{
  const VkResult result{ vkWaitForFences(
    d_->device_.logical(), 1, &d_->fence_, VK_TRUE, timeout) };
  switch (result) {
    case VK_SUCCESS:
    case VK_TIMEOUT:
      return result;
    default:
      Throw("Failed to wait for fences! ({})", result);
  }
}

VkResult Fence::status() const
{
  return vkGetFenceStatus(d_->device_.logical(), d_->fence_);
}
NAMESPACE_END(eldr::vk::wr)
