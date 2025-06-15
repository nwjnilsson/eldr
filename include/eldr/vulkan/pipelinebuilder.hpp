#pragma once
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>

#include <vector>

NAMESPACE_BEGIN(eldr::vk)

/// @brief Builder class for wr::Pipeline, which combines the pipeline layout
/// and pipeline into the same object
class PipelineBuilder {
public:
  PipelineBuilder() { reset(); }

  void reset();

  PipelineBuilder& addDescriptorSetLayout(const wr::DescriptorSetLayout& layout)
  {
    descriptor_layouts_.push_back(layout.vk());
    return *this;
  }

  PipelineBuilder& addVertexAttribute(VkFormat format, uint32_t offset);

  PipelineBuilder& addVertexBinding(uint32_t binding, size_t stride)
  {
    vertex_bindings_.push_back({
      .binding   = binding,
      .stride    = static_cast<uint32_t>(stride),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    });
    return *this;
  }

  PipelineBuilder& addPushConstantRange(VkPushConstantRange pcr)
  {
    push_constant_ranges_.push_back(pcr);
    return *this;
  }
  PipelineBuilder& setShaders(const wr::ShaderModule& vertex_shader,
                              const wr::ShaderModule& fragment_shader);
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
                                   std::string_view  name,
                                   VkPipelineLayoutCreateFlags = 0,
                                   VkPipelineCreateFlags       = 0);

private:
  // Pipeline layout stuff
  std::vector<VkDescriptorSetLayout>             descriptor_layouts_;
  std::vector<VkVertexInputAttributeDescription> vertex_attributes_;
  std::vector<VkVertexInputBindingDescription>   vertex_bindings_;
  std::vector<VkPushConstantRange>               push_constant_ranges_;

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
NAMESPACE_END(eldr::vk)
