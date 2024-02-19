#include <eldr/core/logger.hpp>
#include <eldr/vulkan/renderpass.hpp>
#include <vulkan/vulkan_core.h>

namespace eldr {
namespace vk {
RenderPass::RenderPass(const Device* device, VkFormat image_format)
  : device_(device)
{
  VkAttachmentDescription color_attachment{};
  color_attachment.format         = image_format;
  color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &color_attachment_ref;

  VkSubpassDependency dependency{};
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_ci{};
  render_pass_ci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_ci.attachmentCount = 1;
  render_pass_ci.pAttachments    = &color_attachment;
  render_pass_ci.subpassCount    = 1;
  render_pass_ci.pSubpasses      = &subpass;
  render_pass_ci.dependencyCount = 1;
  render_pass_ci.pDependencies   = &dependency;

  if (vkCreateRenderPass(device_->logical(), &render_pass_ci, nullptr,
                         &render_pass_) != VK_SUCCESS) {
    ThrowVk("Failed to create render pass!");
  }
}

RenderPass::~RenderPass()
{
  if (render_pass_ != VK_NULL_HANDLE)
    vkDestroyRenderPass(device_->logical(), render_pass_, nullptr);
}
} // namespace vk
} // namespace eldr
