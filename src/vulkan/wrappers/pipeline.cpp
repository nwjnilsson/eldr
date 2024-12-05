#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

namespace eldr::vk::wr {

Pipeline::Pipeline(const Device& device, std::string_view name,
                   const VkPipelineLayoutCreateInfo& pipeline_layout_ci)
  : device_(device), name_(name)
{
  if (const auto result = vkCreatePipelineLayout(
        device_.logical(), &pipeline_layout_ci, nullptr, &pipeline_layout_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreatePipelineLayout(): ");
}

Pipeline::Pipeline(Pipeline&& other) noexcept
  : device_(other.device_), name_(std::move(other.name_))
{
  pipeline_        = std::exchange(other.pipeline_, VK_NULL_HANDLE);
  pipeline_layout_ = std::exchange(other.pipeline_layout_, VK_NULL_HANDLE);
}

Pipeline::~Pipeline()
{
  if (pipeline_layout_ != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(device_.logical(), pipeline_layout_, nullptr);
  if (pipeline_ != VK_NULL_HANDLE)
    vkDestroyPipeline(device_.logical(), pipeline_, nullptr);
}

GraphicsPipeline::GraphicsPipeline(
  const Device& device, std::string_view name,
  const VkPipelineLayoutCreateInfo&   pipeline_layout_ci,
  const VkGraphicsPipelineCreateInfo& pipeline_ci)
  : Pipeline(device, name, pipeline_layout_ci)
{
  if (const auto result = vkCreateGraphicsPipelines(
        device_.logical(), nullptr, 1, &pipeline_ci, nullptr, &pipeline_);
      result != VK_SUCCESS) {
    ThrowVk(result, "vkCreateGraphicsPipelines(): ");
  }
}

} // namespace eldr::vk::wr
