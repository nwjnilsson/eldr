#pragma once

#include <eldr/vulkan/common.hpp>
#include <span>

namespace eldr::vk::wr {

class RenderPass {

public:
  RenderPass() = default;
  RenderPass(const Device&                      device,
             std::span<VkAttachmentDescription> attachments,
             const VkSubpassDescription&        subpass_description,
             const VkSubpassDependency&         subpass_dependency);

  [[nodiscard]] VkRenderPass vk() const;

private:
  class RenderPassImpl;
  std::shared_ptr<RenderPassImpl> sampler_data_;
};
} // namespace eldr::vk::wr
