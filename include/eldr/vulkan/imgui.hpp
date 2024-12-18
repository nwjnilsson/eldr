#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/core/math.hpp>
#include <eldr/vulkan/wrappers/descriptorsetlayout.hpp>
#include <eldr/vulkan/wrappers/device.hpp>
#include <eldr/vulkan/wrappers/shader.hpp>
#include <eldr/vulkan/wrappers/swapchain.hpp>
#include <eldr/vulkan/wrappers/texture.hpp>

#include <imgui.h>

class GLFWwindow;
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

  void update(DescriptorAllocator& descriptors);

private:
  const wr::Device    device_;
  const wr::Swapchain swapchain_;
  Logger              log_{ requestLogger("imgui-overlay") };
  float               scale_{ 1.0f };

  BufferResource* index_buffer_{ nullptr };
  BufferResource* vertex_buffer_{ nullptr };
  GraphicsStage*  stage_{ nullptr };

  wr::Texture imgui_texture_;
  wr::Shader  vertex_shader_;
  wr::Shader  fragment_shader_;

  wr::DescriptorSetLayout set_layout_;

  struct PushConstantBlock {
    Vec2f scale;
    Vec2f translate;
  } push_const_block_;
};

} // namespace eldr::vk
