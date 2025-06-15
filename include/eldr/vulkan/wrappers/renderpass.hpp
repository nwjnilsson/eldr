#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)

class RenderPass : public VkDeviceObject<VkRenderPass> {
  using Base = VkDeviceObject<VkRenderPass>;

public:
  EL_VK_IMPORT_DEFAULTS(RenderPass)
  RenderPass(std::string_view                   name,
             const Device&                      device,
             std::span<VkAttachmentDescription> attachments,
             const VkSubpassDescription&        subpass_description,
             const VkSubpassDependency&         subpass_dependency);
};
NAMESPACE_END(eldr::vk::wr)
