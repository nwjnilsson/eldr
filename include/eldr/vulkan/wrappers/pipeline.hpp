#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <vector>

namespace eldr::vk::wr {

class Pipeline {
  // TODO: currently only support graphic pipelines
public:
  Pipeline(const Device&, const Swapchain&, const GraphicsStage&,
           const PhysicalGraphicsStage&,
           const std::vector<VkVertexInputAttributeDescription>&,
           const std::vector<VkVertexInputBindingDescription>&);
  ~Pipeline();

  VkPipeline       get() const { return pipeline_; }
  VkPipelineLayout layout() const { return layout_; }

private:
  const Device& device_;

  VkPipeline       pipeline_{ VK_NULL_HANDLE };
  VkPipelineLayout layout_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
