#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// PipelineImpl
//------------------------------------------------------------------------------
class Pipeline::PipelineImpl {
public:
  PipelineImpl(const Device&                     device,
               const VkPipelineLayoutCreateInfo& layout_ci,
               VkGraphicsPipelineCreateInfo&     pipeline_ci);
  PipelineImpl(const Device&                     device,
               const VkPipelineLayoutCreateInfo& layout_ci,
               VkComputePipelineCreateInfo&      pipeline_ci);
  ~PipelineImpl();

  void createPipelineLayout(const VkPipelineLayoutCreateInfo& layout_ci);
  const Device&    device_;
  VkPipeline       pipeline_{ VK_NULL_HANDLE };
  VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
};

// Graphics Pipeline creation
Pipeline::PipelineImpl::PipelineImpl(
  const Device&                     device,
  const VkPipelineLayoutCreateInfo& layout_ci,
  VkGraphicsPipelineCreateInfo&     pipeline_ci)
  : device_(device)
{
  createPipelineLayout(layout_ci);
  pipeline_ci.layout = pipeline_layout_;
  if (const VkResult result{ vkCreateGraphicsPipelines(
        device_.logical(), nullptr, 1, &pipeline_ci, nullptr, &pipeline_) };
      result != VK_SUCCESS)
    Throw("Failed to create graphics pipeline! ({})", result);
}

// Compute Pipeline creation
Pipeline::PipelineImpl::PipelineImpl(
  const Device&                     device,
  const VkPipelineLayoutCreateInfo& layout_ci,
  VkComputePipelineCreateInfo&      pipeline_ci)
  : device_(device)
{
  createPipelineLayout(layout_ci);
  pipeline_ci.layout = pipeline_layout_;
  if (const VkResult result{ vkCreateComputePipelines(
        device_.logical(), nullptr, 1, &pipeline_ci, nullptr, &pipeline_) };
      result != VK_SUCCESS)
    Throw("Failed to create compute pipelines! ({})", result);
}

Pipeline::PipelineImpl::~PipelineImpl()
{
  vkDestroyPipelineLayout(device_.logical(), pipeline_layout_, nullptr);
  vkDestroyPipeline(device_.logical(), pipeline_, nullptr);
}

void Pipeline::PipelineImpl::createPipelineLayout(
  const VkPipelineLayoutCreateInfo& layout_ci)
{
  if (const VkResult result{ vkCreatePipelineLayout(
        device_.logical(), &layout_ci, nullptr, &pipeline_layout_) };
      result != VK_SUCCESS)
    Throw("Failed to create pipeline layout! ({})", result);
}

//------------------------------------------------------------------------------
// Pipeline
//------------------------------------------------------------------------------
Pipeline::Pipeline()                      = default;
Pipeline::Pipeline(Pipeline&&) noexcept   = default;
Pipeline::~Pipeline()                     = default;
Pipeline& Pipeline::operator=(Pipeline&&) = default;

Pipeline::Pipeline(const Device&                     device,
                   std::string_view                  name,
                   const VkPipelineLayoutCreateInfo& layout_ci,
                   VkGraphicsPipelineCreateInfo&     pipeline_ci)
  : name_(name),
    d_(std::make_unique<PipelineImpl>(device, layout_ci, pipeline_ci))
{
}

Pipeline::Pipeline(const Device&                     device,
                   std::string_view                  name,
                   const VkPipelineLayoutCreateInfo& layout_ci,
                   VkComputePipelineCreateInfo&      pipeline_ci)
  : name_(name),
    d_(std::make_unique<PipelineImpl>(device, layout_ci, pipeline_ci))
{
}

VkPipeline       Pipeline::vk() const { return d_->pipeline_; }
VkPipelineLayout Pipeline::layout() const { return d_->pipeline_layout_; }
} // namespace eldr::vk::wr
