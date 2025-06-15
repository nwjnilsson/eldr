#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/fence.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(Fence)
EL_VK_IMPL_DESTRUCTOR(Fence)

Fence::Fence(std::string_view name, const Device& device) : Base(name, device)
{
  VkFenceCreateInfo fence_ci{};
  fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (const VkResult result{
        vkCreateFence(device.logical(), &fence_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create fence! ({})", result);
}

VkResult Fence::reset() const
{
  return vkResetFences(device().logical(), 1, &object_);
}

VkResult Fence::wait(uint64_t timeout) const
{
  const VkResult result{ vkWaitForFences(
    device().logical(), 1, &object_, VK_TRUE, timeout) };
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
  return vkGetFenceStatus(device().logical(), object_);
}
NAMESPACE_END(eldr::vk::wr)
