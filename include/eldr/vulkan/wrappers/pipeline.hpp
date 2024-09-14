#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk::wr {

class Pipeline {
public:
  Pipeline(Device&, const Swapchain&, VkDescriptorSetLayout);
  ~Pipeline();

  VkPipeline       get() const { return pipeline_; }
  VkPipelineLayout layout() const { return layout_; }

private:
  Device& device_;

  VkPipeline       pipeline_;
  VkPipelineLayout layout_;
};
} // namespace eldr::vk::wr
