#pragma once

#include <gui/vulkan-backend.hpp>

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

  vk_wrapper::VkWrapper vk_wrapper_;

  GLFWwindow* window_;
};
} // namespace eldr
