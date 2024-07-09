#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <string>

namespace eldr {

class Window {
public:
  Window() = delete;
  Window(int width, int height, std::string name);
  ~Window();

  void display();
  inline void resize() { resized_ = true; }

  inline bool        shouldClose() { return glfwWindowShouldClose(window_); }
  inline GLFWwindow* getGLFWwindow() { return window_; };
  inline std::vector<const char*>& getExtensions() { return extensions_; }

private:
  const int   width_;
  const int   height_;
  const std::string window_name_;
  bool resized_;

  std::vector<const char*> extensions_;

  GLFWwindow*                        window_;
};
} // namespace eldr
