#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <string>
#include <vector>

namespace eldr::vk::wr {

class Framebuffer {
public:
  Framebuffer(const Device& device, VkRenderPass render_pass,
              const std::vector<VkImageView>& attachments,
              const Swapchain&                swapchain);

  Framebuffer(const Framebuffer&) = delete;
  Framebuffer(Framebuffer&&) noexcept;

  ~Framebuffer();

  Framebuffer& operator=(const Framebuffer&) = delete;
  Framebuffer& operator=(Framebuffer&&)      = delete;

  [[nodiscard]] VkFramebuffer get() const { return framebuffer_; }

private:
  const Device& device_;
  VkFramebuffer framebuffer_{ VK_NULL_HANDLE };
  std::string   name_;
};

} // namespace eldr::vk::wr
