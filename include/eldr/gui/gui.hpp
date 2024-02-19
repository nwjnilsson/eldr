#pragma once

#include <eldr/vulkan/vulkan.hpp>

#include <string>

namespace eldr {

class EldrGUI {
public:
  EldrGUI(int width, int height, std::string name)
    : width_{ width }, height_{ height }, window_name_{ name } {};

  //~EldrGUI();

  // EldrGUI(const EldrGUI &)            = delete;
  // EldrGUI &operator=(const EldrGUI &) = delete;

  void display();
  void init();
  void terminate();

  inline bool        shouldClose() { return glfwWindowShouldClose(window_); }
  inline GLFWwindow* getGLFWwindow() { return window_; };

private:
  const int   width_;
  const int   height_;
  std::string window_name_;

  std::unique_ptr<vk::VulkanWrapper> vk_wrapper_;

  GLFWwindow* window_;
};
} // namespace eldr
