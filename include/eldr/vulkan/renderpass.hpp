#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/device.hpp>

namespace eldr {
namespace vk {

class RenderPass {
public:
  RenderPass(const Device*, VkFormat);
  ~RenderPass();

  const VkRenderPass& get() const { return render_pass_; }

private:
  const Device* device_;

  VkRenderPass render_pass_;
};
} // namespace vk
} // namespace eldr
