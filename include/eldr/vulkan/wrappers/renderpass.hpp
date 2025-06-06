#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <span>

namespace eldr::vk::wr {

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
} // namespace eldr::vk::wr
