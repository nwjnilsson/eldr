#pragma once

#include <GLFW/glfw3.h>
#include <gui/vulkan-backend.hpp>
#include <imgui.h>
#include <memory>
#include <string.h>
#include <vector>

namespace eldr {

class EldrGUI {
public:
  EldrGUI(int width, int height, std::string name);
  ~EldrGUI();

  // EldrGUI(const EldrGUI &)            = delete;
  // EldrGUI &operator=(const EldrGUI &) = delete;

  inline bool shouldClose() { return glfwWindowShouldClose(window_); }

private:
  void init();
  const int   width_;
  const int   height_;
  std::string window_name_;

  VulkanData vulkan_data_;

  GLFWwindow* window_;
};
} // namespace eldr
