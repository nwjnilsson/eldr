#pragma once

#include <gui/vulkan-backend.hpp>

namespace eldr {

class EldrGUI {
public:
  EldrGUI(int width, int height, std::string name)
    : width_{ width }, height_{ height }, window_name_{ name } {};

  ~EldrGUI();

  // EldrGUI(const EldrGUI &)            = delete;
  // EldrGUI &operator=(const EldrGUI &) = delete;

  inline bool shouldClose() { return glfwWindowShouldClose(window_); }
  void        display();
  void        init();
  void        terminate();

private:
  const int   width_;
  const int   height_;
  std::string window_name_;

  vk_wrapper::VkWrapper vk_wrapper_;

  // TODO: decide whether this pointer should live here or in vulkan_backend.hpp
  // If it lives in the wrapper, initializing is slightly simpler
  GLFWwindow* window_;
};
} // namespace eldr
