#pragma once
#include <eldr/vulkan/common.hpp>

#include <vector>

// fwd
struct GLFWwindow;

namespace eldr::vk::wr {
class Window {
public:
  Window() = delete;
  Window(uint32_t width, uint32_t height);
  ~Window();

  std::vector<const char*> getExtensions();
  void resize(uint32_t width, uint32_t height) { extent_ = { width, height }; }

  VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
  GLFWwindow* glfw_window_;
  VkExtent2D  extent_;
};

} // namespace eldr::vk::wr
