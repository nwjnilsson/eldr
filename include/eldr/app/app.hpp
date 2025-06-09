#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/app/keyboardmouseinput.hpp>
#include <eldr/app/window.hpp>
#include <eldr/core/stopwatch.hpp>
#include <eldr/render/scene.hpp>

#include <memory>

// -----------------------------------------------------------------------------
// fwd
// -----------------------------------------------------------------------------
namespace eldr::vk {
class VulkanEngine;
}
// -----------------------------------------------------------------------------
namespace eldr::app {
class App {
  const std::filesystem::path model_path = "assets/models/Suzanne.gltf";

public:
  App();
  ~App();

  /// @brief Call glfwSetKeyCallback.
  /// @param window The window that received the event.
  /// @param key The keyboard key that was pressed or released.
  /// @param scancode The system-specific scancode of the key.
  /// @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
  /// @param mods Bit field describing which modifier keys were held down.
  void keyCallback(Window* window, int key, int scancode, int action, int mods);

  /// @brief Call glfwSetCursorPosCallback.
  /// @param window The window that received the event.
  /// @param x_pos The new x-coordinate, in screen coordinates, of the cursor.
  /// @param y_pos The new y-coordinate, in screen coordinates, of the cursor.
  void cursorPositionCallback(Window* window, double x_pos, double y_pos);

  /// @brief Call glfwSetMouseButtonCallback.
  /// @param window The window that received the event.
  /// @param button The mouse button that was pressed or released.
  /// @param action One of GLFW_PRESS or GLFW_RELEASE.
  /// @param mods Bit field describing which modifier keys were held down.
  void mouseButtonCallback(Window* window, int button, int action, int mods);

  /// @brief Call camera's process_mouse_scroll method.
  /// @param window The window that received the event.
  /// @param x_offset The change of x-offset of the mouse wheel.
  /// @param y_offset The change of y-offset of the mouse wheel.
  void mouseScrollCallback(Window* window, double x_offset, double y_offset);

  void run();

private:
  void setupWindowCallbacks();
  void setupInputCallbacks();
  void updateImGui();

public:
  static constexpr uint32_t width  = 1280;
  static constexpr uint32_t height = 720;

private:
  KeyboardMouseInput input_data_;
  Window             window_;

  std::unique_ptr<vk::VulkanEngine> vk_engine_;

  float           frame_time_{};
  core::StopWatch stop_watch_{};
};

} // namespace eldr::app
