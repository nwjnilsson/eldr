#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <vulkan/vulkan_core.h>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// RenderPassImpl
//------------------------------------------------------------------------------
class RenderPass::RenderPassImpl {
public:
  RenderPassImpl(const Device&                 device,
                 const VkRenderPassCreateInfo& render_pass_ci);
  ~RenderPassImpl();
  const Device& device_;
  VkRenderPass  render_pass_{ VK_NULL_HANDLE };
};

RenderPass::RenderPassImpl::RenderPassImpl(
  const Device& device, const VkRenderPassCreateInfo& render_pass_ci)
  : device_(device)
{
  if (const VkResult result{ vkCreateRenderPass(
        device.logical(), &render_pass_ci, nullptr, &render_pass_) };
      result != VK_SUCCESS) {
    Throw("Failed to create render pass! ({})", result);
  }
}

RenderPass::RenderPassImpl::~RenderPassImpl()
{
  vkDestroyRenderPass(device_.logical(), render_pass_, nullptr);
}

//------------------------------------------------------------------------------
// RenderPass
//------------------------------------------------------------------------------
RenderPass::RenderPass(const Device&                      device,
                       std::span<VkAttachmentDescription> attachments,
                       const VkSubpassDescription&        subpass_description,
                       const VkSubpassDependency&         subpass_dependency)
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
  d_ = std::make_unique<RenderPassImpl>(device, render_pass_ci);
}

VkRenderPass RenderPass::vk() const { return d_->render_pass_; }

} // namespace eldr::vk::wr
