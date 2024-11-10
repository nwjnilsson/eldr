#include <eldr/app/window.hpp>
#include <eldr/core/logger.hpp>

#include <GLFW/glfw3.h>

namespace eldr {
namespace {
static void glfwErrorCallback(int error, const char* description)
{
  detail::requestLogger("app")->error("GLFW Error {}: {}", error, description);
}
} // namespace

Window::Window(uint32_t width, uint32_t height) : width_(width), height_(height)
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
  glfw_window_ = glfwCreateWindow(width, height, "Eldr", nullptr, nullptr);
}

Window::~Window()
{
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
}

void Window::setUserPointer(void* user)
{
  glfwSetWindowUserPointer(glfw_window_, user);
}

void Window::setResizeCallback(GLFWframebuffersizefun func)
{
  glfwSetFramebufferSizeCallback(glfw_window_, func);
}

std::vector<const char*> Window::getExtensions()
{
  std::vector<const char*> extensions{};
  uint32_t                 extensions_count = 0;
  const char**             glfw_extensions =
    glfwGetRequiredInstanceExtensions(&extensions_count);
  for (size_t i = 0; i < extensions_count; i++)
    extensions.push_back(glfw_extensions[i]);
  return extensions;
}

void Window::resize(uint32_t width, uint32_t height)
{
  width_  = width;
  height_ = height;
}

std::vector<const char*> Window::instanceExtensions() const
{
  uint32_t     count      = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&count);
  return std::vector<const char*>(extensions, extensions + count);
}

bool Window::shouldClose() const { return glfwWindowShouldClose(glfw_window_); }

void Window::waitForFocus() const
{
  int width  = 0;
  int height = 0;
  glfwGetFramebufferSize(glfw_window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(glfw_window_, &width, &height);
    glfwWaitEvents();
  }
}

} // namespace eldr
