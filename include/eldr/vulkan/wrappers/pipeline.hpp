#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
/// @brief VkPipeline wrapper class. Pipeline manages both VkPipeline and
/// VkPipelineLayout.
///
/// TODO: might make sense to have a ComputePipeline and GraphicsPipeline that
/// inherit from Pipeline
class Pipeline {
public:
  Pipeline();
  Pipeline(const Device&                     device,
           std::string_view                  name,
           const VkPipelineLayoutCreateInfo& layout_ci,
           VkGraphicsPipelineCreateInfo&     pipeline_ci);
  Pipeline(const Device&                     device,
           std::string_view                  name,
           const VkPipelineLayoutCreateInfo& layout_ci,
           VkComputePipelineCreateInfo&      pipeline_ci);
  Pipeline(Pipeline&&) noexcept;
  ~Pipeline();

  Pipeline& operator=(Pipeline&&);

  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] VkPipeline         vk() const;
  [[nodiscard]] VkPipelineLayout   layout() const;

protected:
  std::string name_;
  class PipelineImpl;
  std::unique_ptr<PipelineImpl> d_;
};

// Support for compute pipelines can be added if needed

NAMESPACE_END(eldr::vk::wr)
