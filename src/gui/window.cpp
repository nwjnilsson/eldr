#include <eldr/core/logger.hpp>
#include <eldr/gui/window.hpp>

#include <stdexcept>

namespace eldr {
static void glfwErrorCallback(int error, const char* description)
{
  spdlog::error("GLFW Error %d: %s", error, description);
}
static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
  // Let vulkan wrapper manage resize
  (void) width;
  (void) height;
  auto viewer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  viewer->resize();
}

Window::Window(int width, int height, std::string name)
  : width_{ width }, height_{ height }, window_name_{ name }
{
  // Initialize GLFW
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    Throw("[GLFW]: Failed to initialize");
  }

  // Vulkan pre-check
  if (!glfwVulkanSupported()) {
    Throw("[GLFW]: Vulkan not supported");
  }

  // Create GLFW Window with Vulkan context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ =
    glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

  uint32_t                 extensions_count = 0;
  const char**             glfw_extensions =
    glfwGetRequiredInstanceExtensions(&extensions_count);
  for (size_t i = 0; i < extensions_count; i++)
    extensions_.push_back(glfw_extensions[i]);

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
};

Window::~Window()
{
  glfwDestroyWindow(window_);
  glfwTerminate();
}

} // Namespace eldr
