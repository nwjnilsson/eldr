#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>
#include <vulkan/vulkan_core.h>

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

  CheckVkResult(vkCreateFramebuffer(device_.logical(), &framebuffer_ci, nullptr,
                                    &framebuffer_));
}

Framebuffer::~Framebuffer()
{
  if (framebuffer_ != VK_NULL_HANDLE)
    vkDestroyFramebuffer(device_.logical(), framebuffer_, nullptr);
}
} // namespace eldr::vk::wr
