#include <eldr/core/logger.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <GLFW/glfw3.h>

namespace eldr::vk::wr {

// Create surface
Surface::Surface(Instance& instance, GLFWwindow* window) : instance_(instance)
{
  if (glfwCreateWindowSurface(instance.vkInstance(), window, nullptr,
                              &surface_) != VK_SUCCESS)
    ThrowVk("Failed to create window surface!");
}

Surface::~Surface()
{
  if (surface_ != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance_.vkInstance(), surface_, nullptr);
}

} // namespace eldr::vk::wr
