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
