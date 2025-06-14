#include <eldr/app/app.hpp>
#include <eldr/app/window.hpp>
#include <eldr/core/logger.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr)
NAMESPACE_BEGIN()
static void glfwErrorCallback(const int error, const char* description)
{
  Log(core::Error, "GLFW Error {}: {}", error, description);
}
NAMESPACE_END()

Window::Window(const int width, const int height)
  : width_(width), height_(height)
{
  // Initialize GLFW
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    Throw("Failed to initialize GLFW!");
  }

  // Vulkan pre-check
  if (!glfwVulkanSupported()) {
    Throw("Vulkan is not supported!");
  }

  // Create GLFW Window with Vulkan context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfw_window_ = glfwCreateWindow(width, height, "Eldr", nullptr, nullptr);
  glfwSetWindowUserPointer(glfw_window_, this);
}

Window::~Window()
{
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
}

void Window::setResizeCallback(const ResizeFunc resize_callback)
{
  resize_func_       = resize_callback;
  auto resize_lambda = [](GLFWwindow* window, int width, int height) {
    auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->resize_func_(w, width, height);
    w->width_  = width;
    w->height_ = height;
  };
  glfwSetFramebufferSizeCallback(glfw_window_, resize_lambda);
}

void Window::setTitle(const std::string& title)
{
  Assert(not title.empty());
  glfwSetWindowTitle(glfw_window_, title.c_str());
}

void Window::setKeyboardButtonCallback(const KeyFunc keyboard_button_callback)
{
  key_func_ = keyboard_button_callback;
  auto key_callback =
    [](GLFWwindow* window, int key, int scancode, int action, int mods) {
      auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
      w->key_func_(w, key, scancode, action, mods);
    };
  glfwSetKeyCallback(glfw_window_, key_callback);
}

void Window::setCursorPositionCallback(const CursorPosFunc cursor_pos_callback)
{
  cursor_pos_func_     = cursor_pos_callback;
  auto cursor_callback = [](GLFWwindow* window, double xpos, double ypos) {
    auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    w->cursor_pos_func_(w, xpos, ypos);
  };
  glfwSetCursorPosCallback(glfw_window_, cursor_callback);
}

void Window::setMouseButtonCallback(const MouseButtonFunc mouse_button_callback)
{
  mouse_button_func_ = mouse_button_callback;
  auto button_callback =
    [](GLFWwindow* window, int button, int action, int mods) {
      auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
      w->mouse_button_func_(w, button, action, mods);
    };
  glfwSetMouseButtonCallback(glfw_window_, button_callback);
}

void Window::setMouseScrollCallback(const ScrollFunc mouse_scroll_callback)
{
  scroll_func_ = mouse_scroll_callback;
  auto scroll_callback =
    [](GLFWwindow* window, double xoffset, double yoffset) {
      auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
      w->scroll_func_(w, xoffset, yoffset);
    };
  glfwSetScrollCallback(glfw_window_, scroll_callback);
}

void Window::show() { glfwShowWindow(glfw_window_); }

void                     Window::poll() { glfwPollEvents(); }
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

// void Window::resize(uint32_t width, uint32_t height)
// {
//   width_  = width;
//   height_ = height;
// }

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

NAMESPACE_END(eldr)
