#pragma once
#include <eldr/vulkan/common.hpp>

namespace eldr::vk::wr {
class Pipeline {
public:
  Pipeline()                = delete;
  Pipeline(const Pipeline&) = delete;
  virtual ~Pipeline();

  [[nodiscard]] VkPipeline get() const { return pipeline_; }

protected:
  Pipeline(Pipeline&&) noexcept;
  Pipeline(const Device& device, std::string_view name,
           const VkPipelineLayoutCreateInfo& pipeline_layout_ci);

protected:
  const Device& device_;
  std::string   name_;

  VkPipeline       pipeline_{ VK_NULL_HANDLE };
  VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
};

class GraphicsPipeline final : public Pipeline {
public:
  GraphicsPipeline()                = delete;
  GraphicsPipeline(const Pipeline&) = delete;
  inline GraphicsPipeline(GraphicsPipeline&& other) noexcept
    : Pipeline(std::move(other))
  {
  }
  GraphicsPipeline(const Device& device, std::string_view name,
                   const VkPipelineLayoutCreateInfo&   pipeline_layout_ci,
                   const VkGraphicsPipelineCreateInfo& pipeline_ci);
  ~GraphicsPipeline() override = default;
};

// Support for compute pipelines can be added if needed

} // namespace eldr::vk::wr
