#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

NAMESPACE_BEGIN(eldr::vk::wr)

class RenderPass {

public:
  RenderPass() = default;
  RenderPass(const Device&                      device,
             std::span<VkAttachmentDescription> attachments,
             const VkSubpassDescription&        subpass_description,
             const VkSubpassDependency&         subpass_dependency);
  ~RenderPass() = default;

  [[nodiscard]] VkRenderPass vk() const;

private:
  class RenderPassImpl;
  std::unique_ptr<RenderPassImpl> d_;
};
NAMESPACE_END(eldr::vk::wr)
