#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/framebuffer.hpp>
#include <eldr/vulkan/wrappers/renderpass.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk::wr {
//------------------------------------------------------------------------------
// FramebufferImpl
//------------------------------------------------------------------------------
class Framebuffer::FramebufferImpl {
public:
  FramebufferImpl(const Device&                  device,
                  const VkFramebufferCreateInfo& framebuffer_ci);
  ~FramebufferImpl();
  const Device  device_;
  VkFramebuffer framebuffer_{ VK_NULL_HANDLE };
};

Framebuffer::FramebufferImpl::FramebufferImpl(
  const Device& device, const VkFramebufferCreateInfo& framebuffer_ci)
  : device_(device)
{
  if (const auto result = vkCreateFramebuffer(
        device_.logical(), &framebuffer_ci, nullptr, &framebuffer_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateFramebuffer(): ");
}

Framebuffer::FramebufferImpl::~FramebufferImpl()
{
  vkDestroyFramebuffer(device_.logical(), framebuffer_, nullptr);
}

//------------------------------------------------------------------------------
// Framebuffer
//------------------------------------------------------------------------------
Framebuffer::Framebuffer(const Device& device, const RenderPass& render_pass,
                         const std::vector<VkImageView>& attachments,
                         const Swapchain&                swapchain)
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
  fb_data_ = std::make_shared<FramebufferImpl>(device, framebuffer_ci);
}

VkFramebuffer Framebuffer::get() const { return fb_data_->framebuffer_; }
} // namespace eldr::vk::wr
