#pragma once
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
/// @brief VkPipeline wrapper class. Pipeline manages both VkPipeline and
/// VkPipelineLayout.
///
/// TODO: might make sense to have a ComputePipeline and GraphicsPipeline that
/// inherit from Pipeline
class Pipeline : public VkDeviceObject<VkPipeline> {
  using Base = VkDeviceObject<VkPipeline>;

public:
  EL_VK_IMPORT_DEFAULTS(Pipeline)
  Pipeline(std::string_view                  name,
           const Device&                     device,
           const VkPipelineLayoutCreateInfo& layout_ci,
           VkGraphicsPipelineCreateInfo&     pipeline_ci);
  Pipeline(std::string_view                  name,
           const Device&                     device,
           const VkPipelineLayoutCreateInfo& layout_ci,
           VkComputePipelineCreateInfo&      pipeline_ci);

  [[nodiscard]] VkPipelineLayout layout() const { return pipeline_layout_; }

private:
  void createPipelineLayout(const VkPipelineLayoutCreateInfo& layout_ci);

protected:
  VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
};

// Support for compute pipelines can be added if needed

NAMESPACE_END(eldr::vk::wr)
