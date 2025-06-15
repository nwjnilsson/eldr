#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)
//------------------------------------------------------------------------------
// Pipeline
//------------------------------------------------------------------------------
EL_VK_IMPL_DEFAULTS(Pipeline)
Pipeline::~Pipeline()
{
  if (vk()) {
    vkDestroyPipelineLayout(device().logical(), pipeline_layout_, nullptr);
    vkDestroyPipeline(device().logical(), object_, nullptr);
  }
}

Pipeline::Pipeline(std::string_view                  name,
                   const Device&                     device,
                   const VkPipelineLayoutCreateInfo& layout_ci,
                   VkGraphicsPipelineCreateInfo&     pipeline_ci)
  : Base(name, device)
{
  createPipelineLayout(layout_ci);
  pipeline_ci.layout = pipeline_layout_;
  if (const VkResult result{ vkCreateGraphicsPipelines(
        device.logical(), nullptr, 1, &pipeline_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create graphics pipeline! ({})", result);
}

Pipeline::Pipeline(std::string_view                  name,
                   const Device&                     device,
                   const VkPipelineLayoutCreateInfo& layout_ci,
                   VkComputePipelineCreateInfo&      pipeline_ci)
  : Base(name, device)
{
  createPipelineLayout(layout_ci);
  pipeline_ci.layout = pipeline_layout_;
  if (const VkResult result{ vkCreateComputePipelines(
        device.logical(), nullptr, 1, &pipeline_ci, nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create graphics pipeline! ({})", result);
}

void Pipeline::createPipelineLayout(const VkPipelineLayoutCreateInfo& layout_ci)
{
  if (const VkResult result{ vkCreatePipelineLayout(
        device().logical(), &layout_ci, nullptr, &pipeline_layout_) };
      result != VK_SUCCESS)
    Throw("Failed to create pipeline layout! ({})", result);
}

NAMESPACE_END(eldr::vk::wr)
