#include <eldr/app/app.hpp>
#include <eldr/app/keyboardmouseinput.hpp>
#include <eldr/app/window.hpp>
#include <eldr/render/mesh.hpp>
#include <eldr/vulkan/engine.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace eldr {

// -----------------------------------------------------------------------------
// fwd
// -----------------------------------------------------------------------------

EldrApp::EldrApp()
  : input_data_(std::make_unique<app::input::KeyboardMouseInput>()),
    window_(std::make_unique<Window>(width, height)),
    vk_engine_(std::make_unique<vk::VulkanEngine>(*window_)),
    scene_({ model_path_str, texture_path_str })
{
  setupWindowCallbacks();
  setupInputCallbacks();
}

EldrApp::~EldrApp() {}

void EldrApp::keyCallback(GLFWwindow* /*window*/, int key, int, int action,
                          int /*mods*/)
{
  if (key < 0 || key > GLFW_KEY_LAST) {
    return;
  }

  switch (action) {
    case GLFW_PRESS:
      input_data_->pressKey(key);
      break;
    case GLFW_RELEASE:
      input_data_->releaseKey(key);
      break;
    default:
      break;
  }
}

void EldrApp::cursorPositionCallback(GLFWwindow* /*window*/, double x_pos,
                                     double y_pos)
{
  input_data_->setCursorPos(x_pos, y_pos);
}

void EldrApp::mouseButtonCallback(GLFWwindow* /*window*/, int button,
                                  int action, int /*mods*/)
{
  if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
    return;
  }
  switch (action) {
    case GLFW_PRESS:
      input_data_->pressMouseButton(button);
      break;
    case GLFW_RELEASE:
      input_data_->releaseMouseButton(button);
      break;
    default:
      break;
  }
}

void EldrApp::mouseScrollCallback(GLFWwindow* /*window*/, double /*x_offset*/,
                                  double y_offset)
{
  // camera_->change_zoom(static_cast<float>(y_offset));
}

void EldrApp::run()
{
  submitGeometry(scene_.shapes());
  while (!window_->shouldClose()) {
    glfwPollEvents();
    // vk_engine_->newFrame();
    updateImGui();
    vk_engine_->drawFrame();
    frame_time_ = stop_watch_.seconds();
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
    app->vk_engine_->swapchain_invalidated_ = true;
  };
  window_->setResizeCallback(resize_lambda);
}

void EldrApp::setupInputCallbacks()
{

  auto lambda_key_callback = [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    auto* app = static_cast<EldrApp*>(glfwGetWindowUserPointer(window));
    app->keyCallback(window, key, scancode, action, mods);
  };
  window_->setKeyboardButtonCallback(lambda_key_callback);

  auto lambda_cursor_position_callback = [](GLFWwindow* window, double xpos,
                                            double ypos) {
    auto* app = static_cast<EldrApp*>(glfwGetWindowUserPointer(window));
    app->cursorPositionCallback(window, xpos, ypos);
  };
  window_->setCursorPositionCallback(lambda_cursor_position_callback);

  auto lambda_mouse_button_callback = [](GLFWwindow* window, int button,
                                         int action, int mods) {
    auto* app = static_cast<EldrApp*>(glfwGetWindowUserPointer(window));
    app->mouseButtonCallback(window, button, action, mods);
  };
  window_->setMouseButtonCallback(lambda_mouse_button_callback);

  auto lambda_mouse_scroll_callback = [](GLFWwindow* window, double xoffset,
                                         double yoffset) {
    auto* app = static_cast<EldrApp*>(glfwGetWindowUserPointer(window));
    app->mouseScrollCallback(window, xoffset, yoffset);
  };
  window_->setMouseScrollCallback(lambda_mouse_scroll_callback);
}

void EldrApp::updateImGui()
{

  auto cursor_pos = input_data_->cursorPos();

  ImGuiIO& io     = ImGui::GetIO();
  io.DeltaTime    = frame_time_;
  io.MousePos     = ImVec2(static_cast<float>(cursor_pos[0]),
                           static_cast<float>(cursor_pos[1]));
  io.MouseDown[0] = input_data_->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
  io.MouseDown[1] = input_data_->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
  io.MouseDown[2] = input_data_->isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);

  window_->setTitle(fmt::format("Eldr - {} - {} fps", vk_engine_->deviceName(),
                                std::round(io.Framerate)));
  static bool show_demo_window = true;
  vk_engine_->updateImGui([&]() {
    if (show_demo_window) {
      // ImGui::Text("io.WantCaptureMouse = %d", io.WantCaptureMouse);
      ImGui::ShowDemoWindow(&show_demo_window);
    }
  });
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
} // namespace eldr
