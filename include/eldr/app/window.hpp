#pragma once
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

// fwd
struct GLFWwindow;

namespace eldr {
class Window {

public:
  Window() = delete;
  Window(uint32_t width, uint32_t height);
  ~Window();

  void setUserPointer(void* user);
  void setResizeCallback(GLFWframebuffersizefun resize_func);

  /// @brief Change the window title.
  /// @param title The new title of the window.
  void setTitle(const std::string& title);

  /// @brief Call glfwSetKeyCallback.
  /// @param key_input_callback The keyboard input callback.
  void setKeyboardButtonCallback(GLFWkeyfun keyboard_button_callback);

  /// @brief Call glfwSetCursorPosCallback.
  /// @param cursor_pos_callback They cursor position callback.
  void setCursorPositionCallback(GLFWcursorposfun cursor_pos_callback);

  /// @brief Call glfwSetMouseButtonCallback.
  /// @param mouse_button_callback The mouse button callback.
  void setMouseButtonCallback(GLFWmousebuttonfun mouse_button_callback);

  /// @brief Call glfwSetScrollCallback.
  /// @param mouse_scroll_callback The mouse scroll callback.
  void setMouseScrollCallback(GLFWscrollfun mouse_scroll_callback);

  /// @brief Call glfwShowWindow.
  void show();

  /// @brief Call glfwPollEvents.
  static void poll();

  std::vector<const char*> getExtensions();
  void                     resize(uint32_t width, uint32_t height);
  /// Wait until the window is not minimized anymore
  void waitForFocus() const;

  [[nodiscard]] uint32_t                 width() const { return width_; }
  [[nodiscard]] uint32_t                 height() const { return height_; }
  [[nodiscard]] bool                     shouldClose() const;
  [[nodiscard]] std::vector<const char*> instanceExtensions() const;
  [[nodiscard]] GLFWwindow* glfwWindow() const { return glfw_window_; }

private:
  GLFWwindow* glfw_window_;
  uint32_t    width_{};
  uint32_t    height_{};
};

} // namespace eldr
