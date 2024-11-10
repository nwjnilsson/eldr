#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/common.hpp>

#include <imgui.h>

#include <memory>
#include <vector>

namespace eldr::vk {

class ImGuiOverlay {
  ELDR_IMPORT_CORE_TYPES();

public:
  ImGuiOverlay() = delete;
  ImGuiOverlay(const wr::Device&, const wr::Swapchain&, RenderGraph*,
               TextureResource* back_buffer);
  ImGuiOverlay(ImGuiOverlay&&)      = delete;
  ImGuiOverlay(const ImGuiOverlay&) = delete;
  ~ImGuiOverlay();

  void update();

private:
  const wr::Device&               device_;
  const wr::Swapchain&            swapchain_;
  std::shared_ptr<spdlog::logger> log_{};
  float                           scale_{ 1.0f };

  BufferResource* index_buffer_{ nullptr };
  BufferResource* vertex_buffer_{ nullptr };
  GraphicsStage*  stage_{ nullptr };

  std::unique_ptr<wr::GpuTexture>         imgui_texture_;
  std::unique_ptr<wr::Shader>             vertex_shader_;
  std::unique_ptr<wr::Shader>             fragment_shader_;
  std::unique_ptr<wr::ResourceDescriptor> descriptor_;
  std::vector<std::uint32_t>              index_data_;
  std::vector<ImDrawVert>                 vertex_data_;

  struct PushConstantBlock {
    Vec2f scale;
    Vec2f translate;
  } push_const_block_;
};

} // namespace eldr::vk
