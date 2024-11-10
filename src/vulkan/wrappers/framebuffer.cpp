#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk::wr {
Framebuffer::Framebuffer(const Device& device, const RenderPass& render_pass,
                         const std::vector<VkImageView>& attachments,
                         const Swapchain&                swapchain)
  : device_(device)
{

  VkFramebufferCreateInfo framebuffer_ci{
    .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .renderPass      = render_pass.get(),
    .attachmentCount = static_cast<uint32_t>(attachments.size()),
    .pAttachments    = attachments.data(),
    .width           = swapchain.extent().width,
    .height          = swapchain.extent().height,
    .layers          = 1,
  };

  if (const auto result = vkCreateFramebuffer(
        device_.logical(), &framebuffer_ci, nullptr, &framebuffer_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateFramebuffer(): ");
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept : device_(other.device_)
{
  framebuffer_ = std::exchange(other.framebuffer_, VK_NULL_HANDLE);
  name_        = std::exchange(other.name_, "");
}

Framebuffer::~Framebuffer()
{
  if (framebuffer_ != VK_NULL_HANDLE)
    vkDestroyFramebuffer(device_.logical(), framebuffer_, nullptr);
}
} // namespace eldr::vk::wr
