#pragma once
#include <eldr/vulkan/wrappers/buffer.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/image.hpp>
#include <eldr/vulkan/wrappers/pipeline.hpp>
#include <eldr/vulkan/wrappers/sampler.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>

#include <imgui.h>

class GLFWwindow;
namespace eldr::vk {

class ImGuiOverlay {
  ELDR_IMPORT_CORE_TYPES();

public:
  ImGuiOverlay() = delete;
  ImGuiOverlay(const wr::Device&, const wr::Swapchain&, RenderGraph*);
  ImGuiOverlay(const ImGuiOverlay&)     = delete;
  ImGuiOverlay(ImGuiOverlay&&) noexcept = delete;
  ~ImGuiOverlay();

  void update(DescriptorAllocator& descriptors);

private:
  void buildPipeline();

private:
  const wr::Device&    device_;
  const wr::Swapchain& swapchain_;
  float                scale_{ 1.0f };

  // BufferResource* ibuffer_{ nullptr };
  // BufferResource* vbuffer_{ nullptr };
  GraphicsStage* stage_{ nullptr };
  struct FrameData;
  std::vector<FrameData> frames_in_flight;
  uint32_t               frame_index_{ 0 };

  wr::Image               imgui_texture_;
  wr::Sampler             font_sampler_;
  wr::Shader              vertex_shader_;
  wr::Shader              fragment_shader_;
  wr::DescriptorSetLayout imgui_layout_;
  wr::Pipeline            imgui_pipeline_;

  struct PushConstantBlock {
    Vec2f scale;
    Vec2f translate;
  } push_const_block_;
};

} // namespace eldr::vk
