#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

namespace eldr::vk::wr {

class RenderPass {
public:
  RenderPass(Device&, VkFormat, VkSampleCountFlagBits);
  ~RenderPass();

  VkRenderPass get() const { return render_pass_; }

private:
  const Device* device_;

  VkRenderPass render_pass_;
};
} // namespace eldr::vk::wr
