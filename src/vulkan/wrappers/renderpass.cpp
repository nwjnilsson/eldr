#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>

namespace eldr::vk::wr {
RenderPass::RenderPass(const Device&                               device,
                       const std::vector<VkAttachmentDescription>& attachments,
                       const std::vector<VkAttachmentReference>&   resolve_refs,
                       const std::vector<VkAttachmentReference>& offscreen_refs,
                       const std::vector<VkAttachmentReference>& depth_refs)
  : device_(device)
{
  // VkAttachmentDescription color_attachment{};
  // color_attachment.format         = image_format;
  // color_attachment.samples        = num_samples;
  // color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  // color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  // color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  // color_attachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // VkAttachmentReference color_attachment_ref{};
  // color_attachment_ref.attachment = 0;
  // color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // VkAttachmentDescription depth_attachment{};
  // depth_attachment.format         = device_.findDepthFormat();
  // depth_attachment.samples        = num_samples;
  // depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  // depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  // depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  // depth_attachment.finalLayout =
  //   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  // VkAttachmentReference depth_attachment_ref{};
  // depth_attachment_ref.attachment = 1;
  // depth_attachment_ref.layout =
  //   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // VkAttachmentDescription color_attachment_resolve{};
  // color_attachment_resolve.format         = image_format;
  // color_attachment_resolve.samples        = VK_SAMPLE_COUNT_1_BIT;
  // color_attachment_resolve.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  // color_attachment_resolve.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  // color_attachment_resolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  // color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // color_attachment_resolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  // color_attachment_resolve.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // VkAttachmentReference color_attachment_resolve_ref{};
  // color_attachment_resolve_ref.attachment = 2;
  // color_attachment_resolve_ref.layout =
  //   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // std::array<VkAttachmentDescription, 3> attachments = {
  //   color_attachment, depth_attachment, color_attachment_resolve
  // };

  const VkSubpassDescription subpass_description{
    .flags                = 0,
    .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments    = nullptr,
    .colorAttachmentCount = static_cast<std::uint32_t>(offscreen_refs.size()),
    .pColorAttachments    = offscreen_refs.data(),
    .pResolveAttachments  = resolve_refs.data(),
    .pDepthStencilAttachment =
      !depth_refs.empty() ? depth_refs.data() : nullptr,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments    = nullptr,
  };

  const VkSubpassDependency subpass_dependency{
    .srcSubpass   = VK_SUBPASS_EXTERNAL,
    .dstSubpass   = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    .dependencyFlags = 0,
  };
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
  if (const auto result = vkCreateRenderPass(device_.logical(), &render_pass_ci,
                                             nullptr, &render_pass_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateRenderPass(): ");
  }
}

RenderPass::~RenderPass()
{
  if (render_pass_ != VK_NULL_HANDLE)
    vkDestroyRenderPass(device_.logical(), render_pass_, nullptr);
}
} // namespace eldr::vk::wr
