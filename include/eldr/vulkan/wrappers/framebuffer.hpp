#pragma once
#include <eldr/vulkan/vulkan.hpp>

#include <vector>

namespace eldr::vk::wr {

class Framebuffer {
public:
  Framebuffer() = default;
  Framebuffer(const Device&                   device,
              const RenderPass&               render_pass,
              const std::vector<VkImageView>& attachments,
              const Swapchain&                swapchain);

  [[nodiscard]] VkFramebuffer vk() const;

private:
  // std::string name_;
  class FramebufferImpl;
  std::shared_ptr<FramebufferImpl> d_;
};

} // namespace eldr::vk::wr
