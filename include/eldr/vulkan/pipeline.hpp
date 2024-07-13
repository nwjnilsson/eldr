#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/descriptorsetlayout.hpp>
#include <eldr/vulkan/device.hpp>
#include <eldr/vulkan/swapchain.hpp>

namespace eldr {
namespace vk {

class Pipeline {
public:
  Pipeline(const Device*, const Swapchain&, const RenderPass&,
           const DescriptorSetLayout&, VkSampleCountFlagBits);
  ~Pipeline();

  const VkPipeline&       get() const { return pipeline_; }
  const VkPipelineLayout& layout() const { return layout_; }

private:
  const Device* device_;

  VkPipeline       pipeline_;
  VkPipelineLayout layout_;
};
} // namespace vk
} // namespace eldr
