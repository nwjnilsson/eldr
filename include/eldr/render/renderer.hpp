#pragma once

#include <eldr/gui/window.hpp>
#include <eldr/render/fwd.hpp>
#include <eldr/vulkan/vulkan.hpp>

#include <string>

namespace eldr {

class Renderer {
public:
  Renderer() = delete;
  Renderer(int width, int height);
  ~Renderer();

  void display();

  inline bool running() { return !window_.shouldClose(); }
  void submitGeometry(const std::vector<Shape*>& shapes);
  inline void resize() { vk_engine_->framebuffer_resized_ = true; }

private:
  Window                             window_;
  std::unique_ptr<vk::VulkanEngine> vk_engine_;
};
} // namespace eldr
