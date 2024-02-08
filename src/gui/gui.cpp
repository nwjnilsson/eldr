#include <eldr/core/logger.hpp>
#include <eldr/gui/gui.hpp>
#include <eldr/render/vulkan-wrapper.hpp>

#include <imgui.h>
#include <stdexcept>

namespace eldr {
static void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s", error, description);
}

void EldrGUI::display() { vk_wrapper_.drawFrame(); }

void EldrGUI::init()
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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ =
    glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

  std::vector<const char*> extensions;
  uint32_t                 extensions_count = 0;
  const char**             glfw_extensions =
    glfwGetRequiredInstanceExtensions(&extensions_count);
  for (size_t i = 0; i < extensions_count; i++)
    extensions.push_back(glfw_extensions[i]);

  // Initialize Vulkan
  render::VkWrapperInitInfo info{ .window              = window_,
                                  .instance_extensions = extensions };
  vk_wrapper_.init(info);
}

void EldrGUI::terminate()
{
  glfwDestroyWindow(window_);
  glfwTerminate();
}

} // Namespace eldr
