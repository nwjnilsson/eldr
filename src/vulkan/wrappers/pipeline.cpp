#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// PipelineImpl
//------------------------------------------------------------------------------
class Pipeline::PipelineImpl {
public:
  PipelineImpl(const Device&                       device,
               const VkPipelineLayoutCreateInfo&   layout_ci,
               const VkGraphicsPipelineCreateInfo& pipeline_ci);
  PipelineImpl(const Device&                      device,
               const VkPipelineLayoutCreateInfo&  layout_ci,
               const VkComputePipelineCreateInfo& pipeline_ci);
  ~PipelineImpl();

  void createPipelineLayout(const VkPipelineLayoutCreateInfo& layout_ci);
  const Device&    device_;
  VkPipeline       pipeline_{ VK_NULL_HANDLE };
  VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
};

// Graphics Pipeline creation
Pipeline::PipelineImpl::PipelineImpl(
  const Device& device, const VkPipelineLayoutCreateInfo& layout_ci,
  const VkGraphicsPipelineCreateInfo& pipeline_ci)
  : device_(device)
{
  createPipelineLayout(layout_ci);
  if (const auto result = vkCreateGraphicsPipelines(
        device_.logical(), nullptr, 1, &pipeline_ci, nullptr, &pipeline_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateGraphicsPipelines(): ");
}

// Compute Pipeline creation
Pipeline::PipelineImpl::PipelineImpl(
  const Device& device, const VkPipelineLayoutCreateInfo& layout_ci,
  const VkComputePipelineCreateInfo& pipeline_ci)
  : device_(device)
{
  createPipelineLayout(layout_ci);
  if (const auto result = vkCreateComputePipelines(
        device_.logical(), nullptr, 1, &pipeline_ci, nullptr, &pipeline_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreateComputePipelines(): ");
}

Pipeline::PipelineImpl::~PipelineImpl()
{
  vkDestroyPipelineLayout(device_.logical(), pipeline_layout_, nullptr);
  vkDestroyPipeline(device_.logical(), pipeline_, nullptr);
}

void Pipeline::PipelineImpl::createPipelineLayout(
  const VkPipelineLayoutCreateInfo& layout_ci)
{
  if (const auto result = vkCreatePipelineLayout(device_.logical(), &layout_ci,
                                                 nullptr, &pipeline_layout_);
      result != VK_SUCCESS)
    ThrowVk(result, "vkCreatePipelineLayout(): ");
}

//------------------------------------------------------------------------------
// Pipeline
//------------------------------------------------------------------------------
Pipeline::Pipeline(const Device& device, std::string_view name,
                   const VkPipelineLayoutCreateInfo&   layout_ci,
                   const VkGraphicsPipelineCreateInfo& pipeline_ci)
  : name_(name),
    p_data_(std::make_shared<PipelineImpl>(device, layout_ci, pipeline_ci))
{
}

Pipeline::Pipeline(const Device& device, std::string_view name,
                   const VkPipelineLayoutCreateInfo&  layout_ci,
                   const VkComputePipelineCreateInfo& pipeline_ci)
  : name_(name),
    p_data_(std::make_shared<PipelineImpl>(device, layout_ci, pipeline_ci))
{
}

VkPipeline Pipeline::get() const { return p_data_->pipeline_; }
} // namespace eldr::vk::wr
