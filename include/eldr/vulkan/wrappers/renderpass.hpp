#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <vector>

namespace eldr::vk::wr {

class RenderPass {
public:
  RenderPass(const Device&                               device,
             const std::vector<VkAttachmentDescription>& attachments,
             const std::vector<VkAttachmentReference>&   resolve_refs,
             const std::vector<VkAttachmentReference>&   offscreen_refs,
             const std::vector<VkAttachmentReference>&   depth_refs);
  ~RenderPass();

  VkRenderPass get() const { return render_pass_; }

private:
  const Device& device_;

  VkRenderPass render_pass_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
