#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <vulkan/vulkan_core.h>

NAMESPACE_BEGIN(eldr::vk::wr)
EL_VK_IMPL_DEFAULTS(RenderPass)
EL_VK_IMPL_DESTRUCTOR(RenderPass)
RenderPass::RenderPass(std::string_view                   name,
                       const Device&                      device,
                       std::span<VkAttachmentDescription> attachments,
                       const VkSubpassDescription&        subpass_description,
                       const VkSubpassDependency&         subpass_dependency)
  : Base(name, device)
{
  const VkRenderPassCreateInfo render_pass_ci = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
    .pAttachments    = attachments.data(),
    .subpassCount    = 1,
    .pSubpasses      = &subpass_description,
    .dependencyCount = 1,
    .pDependencies   = &subpass_dependency,
  };
  if (const VkResult result{ vkCreateRenderPass(
        device.logical(), &render_pass_ci, nullptr, &object_) };
      result != VK_SUCCESS) {
    Throw("Failed to create render pass! ({})", result);
  }
}

NAMESPACE_END(eldr::vk::wr)
