#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(Framebuffer)
EL_VK_IMPL_DESTRUCTOR(Framebuffer)

Framebuffer::Framebuffer(std::string_view                name,
                         const Device&                   device,
                         const RenderPass&               render_pass,
                         const std::vector<VkImageView>& attachments,
                         const Swapchain&                swapchain)
  : Base(name, device)
{
  VkFramebufferCreateInfo framebuffer_ci{
    .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .renderPass      = render_pass.vk(),
    .attachmentCount = static_cast<uint32_t>(attachments.size()),
    .pAttachments    = attachments.data(),
    .width           = swapchain.extent().width,
    .height          = swapchain.extent().height,
    .layers          = 1,
  };
  if (const VkResult result{ vkCreateFramebuffer(
        device.logical(), &framebuffer_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create framebuffer! ({})", result);
}

NAMESPACE_END(eldr::vk::wr)
