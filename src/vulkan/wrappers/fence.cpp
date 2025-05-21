#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// FenceImpl
//------------------------------------------------------------------------------
class Fence::FenceImpl {
public:
  FenceImpl(const Device& device, const VkFenceCreateInfo& fence_ci);
  ~FenceImpl();
  const Device device_;
  VkFence      fence_{ VK_NULL_HANDLE };
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
Fence::Fence(const Device& device)
{
  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  f_data_        = std::make_shared<FenceImpl>(device, fence_ci);
}

VkFence Fence::get() const { return f_data_->fence_; }

VkResult Fence::reset() const
{
  return vkResetFences(f_data_->device_.logical(), 1, &f_data_->fence_);
}

VkResult Fence::wait(uint64_t timeout) const
{
  const VkResult result{ vkWaitForFences(
    f_data_->device_.logical(), 1, &f_data_->fence_, VK_TRUE, timeout) };
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
  return vkGetFenceStatus(f_data_->device_.logical(), f_data_->fence_);
}
} // namespace eldr::vk::wr
