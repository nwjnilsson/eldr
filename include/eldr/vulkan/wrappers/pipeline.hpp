#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

namespace eldr::vk::wr {

class Pipeline {
public:
  Pipeline(const Device&, const Swapchain&, VkDescriptorSetLayout);
  ~Pipeline();

  VkPipeline       get() const { return pipeline_; }
  VkPipelineLayout layout() const { return layout_; }

private:
  const Device& device_;

  VkPipeline       pipeline_{ VK_NULL_HANDLE };
  VkPipelineLayout layout_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
