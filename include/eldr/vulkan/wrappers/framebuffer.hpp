#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk::wr)

class Framebuffer : public VkDeviceObject<VkFramebuffer> {
  using Base = VkDeviceObject<VkFramebuffer>;

public:
  EL_VK_IMPORT_DEFAULTS(Framebuffer)
  Framebuffer(std::string_view                name,
              const Device&                   device,
              const RenderPass&               render_pass,
              const std::vector<VkImageView>& attachments,
              const Swapchain&                swapchain);
};

NAMESPACE_END(eldr::vk::wr)
