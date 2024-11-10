#include <eldr/app/app.hpp>
#include <eldr/app/window.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/engine.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace eldr {

// -----------------------------------------------------------------------------
// fwd
// -----------------------------------------------------------------------------

EldrApp::EldrApp()
  : window_(std::make_unique<Window>(width, height)),
    vk_engine_(std::make_unique<vk::VulkanEngine>(*window_)),
    scene_({ model_path_str, texture_path_str })
{
  setupWindowCallbacks();
}

EldrApp::~EldrApp() {}

void EldrApp::run()
{
  submitGeometry(scene_.shapes());
  while (!window_->shouldClose()) {
    glfwPollEvents();
    // vk_engine_->newFrame();
    updateImGui();
    vk_engine_->drawFrame();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //   ImGui::UpdatePlatformWindows();
    //   ImGui::RenderPlatformWindowsDefault();
    // }
  }
}

void EldrApp::setupWindowCallbacks()
{
  // ---------------------------------------------------------------------------
  // Set up window
  // ---------------------------------------------------------------------------
  window_->setUserPointer(this);
  auto resize_lambda = [](GLFWwindow* glfw_window, int width, int height) {
    auto app =
      reinterpret_cast<EldrApp*>(glfwGetWindowUserPointer(glfw_window));
    app->window_->resize(static_cast<uint32_t>(width),
                         static_cast<uint32_t>(height));
    app->vk_engine_->framebuffer_resized_ = true;
  };
  window_->setResizeCallback(resize_lambda);
}

void EldrApp::updateImGui()
{
  static bool show_demo_window = true;
  ImGui::NewFrame();
  ImGui::ShowDemoWindow(&show_demo_window);
  // ImGuiIO& io = ImGui::GetIO();
  ImGui::Render();
}

void EldrApp::submitGeometry(const std::vector<Shape*>& shapes)
{
  // Create one big vertex buffer along with index buffer
  // TODO: Shape should contain texture ref probably
  Mesh* mesh = dynamic_cast<Mesh*>(shapes[0]);
  if (mesh != nullptr) {
    vk_engine_->submitGeometry(mesh->vertexPositions(),
                               mesh->vertexTexCoords());
  }
}
} // Namespace eldr
