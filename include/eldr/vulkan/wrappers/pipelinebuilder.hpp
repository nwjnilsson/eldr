#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

#include <vector>

namespace eldr::vk::wr {

/// @brief Builder class for wr::Pipeline, which combines the pipeline layout
/// and pipeline into the same object
class PipelineBuilder {
public:
  PipelineBuilder(const Device& device) : device_(device) { clear(); }

  void clear();

  PipelineBuilder& addDescriptorSetLayout(VkDescriptorSetLayout layout)
  {
    descriptor_layouts_.push_back(layout);
    return *this;
  }
  PipelineBuilder& addPushConstantRange(VkPushConstantRange pcr)
  {
    push_constant_ranges_.push_back(pcr);
    return *this;
  }
  PipelineBuilder& setShaders(const Shader& vertex_shader,
                              const Shader& fragment_shader);
  PipelineBuilder& setInputTopology(VkPrimitiveTopology topology);

  [[nodiscard]] GraphicsPipeline build(std::string_view name);

private:
  const Device& device_;
  // Pipeline layout stuff
  std::vector<VkDescriptorSetLayout> descriptor_layouts_;
  std::vector<VkPushConstantRange>   push_constant_ranges_;

  // Pipeline stuff
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
  VkPipelineInputAssemblyStateCreateInfo       input_assembly_{};
  VkPipelineRasterizationStateCreateInfo       rasterizer_{};
  VkPipelineColorBlendAttachmentState          color_blend_attachment_{};
  VkPipelineMultisampleStateCreateInfo         multisampling_{};
  VkPipelineLayout                             pipeline_layout_{};
  VkPipelineDepthStencilStateCreateInfo        depth_stencil_{};
  VkPipelineRenderingCreateInfo                render_info_{};
  VkFormat                                     color_attachment_format_{};
};
} // namespace eldr::vk::wr
