#pragma once
#include <eldr/app/fwd.hpp>

#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;

NAMESPACE_BEGIN(eldr::app)
class App;
class Window {

public:
  Window() = delete;
  Window(int width, int height);
  ~Window();
  using ResizeFunc = std::function<void(Window*, int width, int height)>;
  using KeyFunc    = std::function<void(
    Window* window, int key, int scancode, int action, int mods)>;
  using CursorPosFunc =
    std::function<void(Window* window, double xpos, double ypos)>;
  using MouseButtonFunc =
    std::function<void(Window* window, int button, int action, int mods)>;
  using ScrollFunc =
    std::function<void(Window* window, double xoffset, double yoffset)>;

  void setResizeCallback(ResizeFunc resize_callback);

  /// @brief Change the window title.
  /// @param title The new title of the window.
  void setTitle(const std::string& title);

  /// @brief Calls glfwSetKeyCallback.
  /// @param key_input_callback The keyboard input callback.
  void setKeyboardButtonCallback(KeyFunc keyboard_button_callback);

  /// @brief Calls glfwSetCursorPosCallback.
  /// @param cursor_pos_callback They cursor position callback.
  void setCursorPositionCallback(CursorPosFunc cursor_pos_callback);

  /// @brief Calls glfwSetMouseButtonCallback.
  /// @param mouse_button_callback The mouse button callback.
  void setMouseButtonCallback(MouseButtonFunc mouse_button_callback);

  /// @brief Calls glfwSetScrollCallback.
  /// @param mouse_scroll_callback The mouse scroll callback.
  void setMouseScrollCallback(ScrollFunc mouse_scroll_callback);

  /// @brief Calls glfwShowWindow.
  void show();

  /// @brief Calls glfwPollEvents.
  static void poll();

  std::vector<const char*> getExtensions();
  // void                     resize(uint32_t width, uint32_t height);
  /// Wait until the window is not minimized anymore
  void waitForFocus() const;

  [[nodiscard]] uint32_t                 width() const { return width_; }
  [[nodiscard]] uint32_t                 height() const { return height_; }
  [[nodiscard]] bool                     shouldClose() const;
  [[nodiscard]] std::vector<const char*> instanceExtensions() const;

  /// @brief Returns a pointer to the underlying GLFW window
  [[nodiscard]] GLFWwindow* glfw() const { return glfw_window_; }

private:
  GLFWwindow*     glfw_window_;
  int             width_{ 0 };
  int             height_{ 0 };
  ResizeFunc      resize_func_;
  KeyFunc         key_func_;
  CursorPosFunc   cursor_pos_func_;
  MouseButtonFunc mouse_button_func_;
  ScrollFunc      scroll_func_;
};

NAMESPACE_END(eldr::app)
