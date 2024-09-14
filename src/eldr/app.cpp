#include <eldr/app.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/engine.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace eldr {

// -----------------------------------------------------------------------------
// fwd
// -----------------------------------------------------------------------------


EldrApp::EldrApp()
  : vk_engine_(std::make_unique<vk::VulkanEngine>(width, height)),
    scene_({ model_path_str, texture_path_str })
{
  vk_engine_->initImGui();
}

EldrApp::~EldrApp()
{
  vk_engine_->shutdownImGui();
}

void EldrApp::resize() const { vk_engine_->framebuffer_resized_ = true; }

void EldrApp::run()
{
  bool show_demo_window = true;
  submitGeometry(scene_.getShapes());
  while (vk_engine_.running()) {
    glfwPollEvents();
    vk_engine_->newFrame();
    ImGui::ShowDemoWindow(&show_demo_window);
    // ImGuiIO& io = ImGui::GetIO();
    ImGui::Render();
    vk_engine_->drawFrame();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //   ImGui::UpdatePlatformWindows();
    //   ImGui::RenderPlatformWindowsDefault();
    // }
  }
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
