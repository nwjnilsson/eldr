#include <eldr/app/app.hpp>
#include <eldr/render/scene.hpp>
#include <eldr/vulkan/engine.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>

NAMESPACE_BEGIN(eldr)

App::App()
  : window_(width, height),
    vk_engine_(std::make_unique<vk::VulkanEngine>(window_))
{
  setupWindowCallbacks();
  setupInputCallbacks();
}
App::~App() = default;

void App::keyCallback(Window* const /*window*/,
                      const int key,
                      const int,
                      const int action,
                      const int /*mods*/)
{
  if (key < 0 || key > GLFW_KEY_LAST) {
    Log(Debug, "Unknown GLFW key ({})", key);
    return;
  }

  switch (action) {
    case GLFW_PRESS:
      input_data_.pressKey(key);
      break;
    case GLFW_RELEASE:
      input_data_.releaseKey(key);
      break;
    default:
      break;
  }
}

void App::cursorPositionCallback(Window* /*window*/,
                                 const double x_pos,
                                 const double y_pos)
{
  input_data_.setCursorPos(x_pos, y_pos);
}

void App::mouseButtonCallback(Window* /*window*/,
                              const int button,
                              const int action,
                              const int /*mods*/)
{
  if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
    return;
  }
  switch (action) {
    case GLFW_PRESS:
      input_data_.pressMouseButton(button);
      break;
    case GLFW_RELEASE:
      input_data_.releaseMouseButton(button);
      break;
    default:
      break;
  }
}

void App::mouseScrollCallback(Window* /*window*/,
                              const double /*x_offset*/,
                              const double y_offset)
{
  // camera_->change_zoom(static_cast<float>(y_offset));
}

void App::run()
{
  using Spectrum = Color<float, 3>;
  auto scene = render::Scene<float, Spectrum>::load(*vk_engine_, { model_path })
                 .value_or(nullptr);
  Assert(scene);

  while (!window_.shouldClose()) {
    glfwPollEvents();
    // vk_engine_->newFrame();
    updateImGui();
    vk_engine_->drawFrame(scene.get());
    frame_time_ = stop_watch_.seconds<float>();
  }
}

void App::setupWindowCallbacks()
{
  auto resize_lambda = [this](Window*, int, int) {
    vk_engine_->invalidateSwapchain();
  };
  window_.setResizeCallback(resize_lambda);
}

void App::setupInputCallbacks()
{
  auto lambda_key_callback =
    [this](Window* window, int key, int scancode, int action, int mods) {
      keyCallback(window, key, scancode, action, mods);
    };
  window_.setKeyboardButtonCallback(lambda_key_callback);

  auto lambda_cursor_position_callback =
    [this](Window* window, double xpos, double ypos) {
      cursorPositionCallback(window, xpos, ypos);
    };
  window_.setCursorPositionCallback(lambda_cursor_position_callback);

  auto lambda_mouse_button_callback =
    [this](Window* window, int button, int action, int mods) {
      mouseButtonCallback(window, button, action, mods);
    };
  window_.setMouseButtonCallback(lambda_mouse_button_callback);

  auto lambda_mouse_scroll_callback =
    [this](Window* window, double xoffset, double yoffset) {
      mouseScrollCallback(window, xoffset, yoffset);
    };
  window_.setMouseScrollCallback(lambda_mouse_scroll_callback);
}

void App::updateImGui()
{

  auto cursor_pos = input_data_.cursorPos();

  ImGuiIO& io     = ImGui::GetIO();
  io.DeltaTime    = frame_time_;
  io.MousePos     = ImVec2(static_cast<float>(cursor_pos[0]),
                       static_cast<float>(cursor_pos[1]));
  io.MouseDown[0] = input_data_.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
  io.MouseDown[1] = input_data_.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
  io.MouseDown[2] = input_data_.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE);

  window_.setTitle(fmt::format(
    "Eldr - {} - {} fps", vk_engine_->deviceName(), std::round(io.Framerate)));
  static bool show_demo_window = true;
  vk_engine_->updateImGui([&]() {
    if (show_demo_window) {
      // ImGui::Text("io.WantCaptureMouse = %d", io.WantCaptureMouse);
      ImGui::ShowDemoWindow(&show_demo_window);
    }
  });
}
NAMESPACE_END(eldr)
