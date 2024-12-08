#pragma once
#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

#include <vector>

namespace eldr::vk {

/// @brief Builder class for wr::Pipeline, which combines the pipeline layout
/// and pipeline into the same object
class PipelineBuilder {
public:
  PipelineBuilder() { reset(); }

  void reset();

  PipelineBuilder& addDescriptorSetLayout(wr::DescriptorSetLayout layout)
  {
    descriptor_layouts_.push_back(layout.get());
    return *this;
  }
  PipelineBuilder& addPushConstantRange(VkPushConstantRange pcr)
  {
    push_constant_ranges_.push_back(pcr);
    return *this;
  }
  PipelineBuilder& setShaders(const wr::Shader& vertex_shader,
                              const wr::Shader& fragment_shader);
  PipelineBuilder& setInputTopology(VkPrimitiveTopology topology);
  PipelineBuilder& setPolygonMode(VkPolygonMode mode);
  PipelineBuilder& setCullMode(VkCullModeFlags mode, VkFrontFace front_face);
  PipelineBuilder& setMultisamplingNone();
  PipelineBuilder& setMultisampling(VkSampleCountFlagBits sample_count,
                                    float min_sample_shading = 0.2f);
  PipelineBuilder& disableBlending();
  PipelineBuilder& enableBlendingAdditive();
  PipelineBuilder& enableBlendingAlphaBlend();
  PipelineBuilder& disableDepthtest();
  PipelineBuilder& enableDepthtest(bool depthWriteEnable, VkCompareOp op);
  PipelineBuilder& setDepthFormat(VkFormat format);
  PipelineBuilder& setColorAttachmentFormat(VkFormat format);

  [[nodiscard]] wr::Pipeline build(const wr::Device& device,
                                   std::string_view  name);

private:
  // Pipeline layout stuff
  std::vector<VkDescriptorSetLayout> descriptor_layouts_;
  std::vector<VkPushConstantRange>   push_constant_ranges_;

  // Pipeline stuff
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
  VkPipelineInputAssemblyStateCreateInfo       input_assembly_;
  VkPipelineRasterizationStateCreateInfo       rasterizer_;
  VkPipelineColorBlendAttachmentState          color_blend_attachment_;
  VkPipelineMultisampleStateCreateInfo         multisampling_;
  VkPipelineDepthStencilStateCreateInfo        depth_stencil_;
  VkPipelineRenderingCreateInfo                render_info_;
  VkFormat color_attachment_format_{ VK_FORMAT_UNDEFINED };
};
} // namespace eldr::vk
