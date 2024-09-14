#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/window.hpp>

#include <GLFW/glfw3.h>

namespace eldr::vk::wr {
namespace {
static void glfwErrorCallback(int error, const char* description)
{
  spdlog::error("GLFW Error %d: %s", error, description);
}

void framebufferResizeCallback(GLFWwindow* glfw_window, int width, int height)
{
  auto window =
    reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
  window->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}
} // namespace

Window::Window(uint32_t width, uint32_t height)
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

  // set resize callback
  glfwSetWindowUserPointer(glfw_window_, this);
  glfwSetFramebufferSizeCallback(glfw_window_, framebufferResizeCallback);
}

Window::~Window()
{
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
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

VkExtent2D
Window::selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
  int width  = 0;
  int height = 0;
  glfwGetFramebufferSize(glfw_window_, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(glfw_window_, &width, &height);
    glfwWaitEvents();
  }

  VkExtent2D extent = { static_cast<uint32_t>(width),
                        static_cast<uint32_t>(height) };
  // Match the resolution of the current window, unless the value is the max
  // of uint32_t
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  else {
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height =
      std::clamp(extent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);
    return extent;
  }
}
} // namespace eldr::vk::wr
