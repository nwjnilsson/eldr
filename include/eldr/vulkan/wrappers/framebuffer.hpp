#pragma once
#include <eldr/vulkan/common.hpp>

#include <vector>

namespace eldr::vk::wr {

class Framebuffer {
public:
  Framebuffer() = default;
  Framebuffer(const Device& device, VkRenderPass render_pass,
              const std::vector<VkImageView>& attachments,
              const Swapchain&                swapchain);

  [[nodiscard]] VkFramebuffer get() const;

private:
  // std::string name_;
  class FramebufferImpl;
  std::shared_ptr<FramebufferImpl> fb_data_;
};

} // namespace eldr::vk::wr
