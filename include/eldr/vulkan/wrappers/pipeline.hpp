#pragma once
#include <eldr/vulkan/vulkan.hpp>

namespace eldr::vk::wr {
/// @brief VkPipeline wrapper class. Pipeline manages both VkPipeline and
/// VkPipelineLayout.
///
/// TODO: might make sense to have a ComputePipeline and GraphicsPipeline that
/// inherit from Pipeline
class Pipeline {
public:
  Pipeline() = default;
  Pipeline(const Device&                     device,
           std::string_view                  name,
           const VkPipelineLayoutCreateInfo& layout_ci,
           VkGraphicsPipelineCreateInfo&     pipeline_ci);
  Pipeline(const Device&                     device,
           std::string_view                  name,
           const VkPipelineLayoutCreateInfo& layout_ci,
           VkComputePipelineCreateInfo&      pipeline_ci);

  [[nodiscard]] VkPipeline       vk() const;
  [[nodiscard]] VkPipelineLayout layout() const;

protected:
protected:
  std::string name_;
  class PipelineImpl;
  std::shared_ptr<PipelineImpl> d_;
};

// Support for compute pipelines can be added if needed

} // namespace eldr::vk::wr
