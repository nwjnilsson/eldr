#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <GLFW/glfw3.h>

namespace eldr::vk::wr {

// Create surface
Surface::Surface(const Instance& instance, GLFWwindow* window)
  : instance_(instance)
{
  if (const auto result =
        glfwCreateWindowSurface(instance.get(), window, nullptr, &surface_);
      result != VK_SUCCESS)
    ThrowVk(result, "glfwCreateWindowSurface(): ");
}

Surface::~Surface()
{
  if (surface_ != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance_.get(), surface_, nullptr);
}

} // namespace eldr::vk::wr
