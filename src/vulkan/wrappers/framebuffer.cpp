#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
//------------------------------------------------------------------------------
// FramebufferImpl
//------------------------------------------------------------------------------
class Framebuffer::FramebufferImpl {
public:
  FramebufferImpl(const Device&                  device,
                  const VkFramebufferCreateInfo& framebuffer_ci);
  ~FramebufferImpl();
  const Device& device_;
  VkFramebuffer framebuffer_{ VK_NULL_HANDLE };
};

Framebuffer::FramebufferImpl::FramebufferImpl(
  const Device& device, const VkFramebufferCreateInfo& framebuffer_ci)
  : device_(device)
{
  if (const VkResult result{ vkCreateFramebuffer(
        device_.logical(), &framebuffer_ci, nullptr, &framebuffer_) };
      result != VK_SUCCESS)
    Throw("Failed to create framebuffer! ({})", result);
}

Framebuffer::FramebufferImpl::~FramebufferImpl()
{
  vkDestroyFramebuffer(device_.logical(), framebuffer_, nullptr);
}

//------------------------------------------------------------------------------
// Framebuffer
//------------------------------------------------------------------------------
Framebuffer::Framebuffer(const Device&                   device,
                         const RenderPass&               render_pass,
                         const std::vector<VkImageView>& attachments,
                         const Swapchain&                swapchain)
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
  d_ = std::make_unique<FramebufferImpl>(device, framebuffer_ci);
}

VkFramebuffer Framebuffer::vk() const { return d_->framebuffer_; }
NAMESPACE_END(eldr::vk::wr)
