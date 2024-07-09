#include <eldr/core/logger.hpp>
#include <eldr/vulkan/surface.hpp>

namespace eldr {
namespace vk {

// Create surface
Surface::Surface(const Instance* instance, GLFWwindow* window) : instance_(instance)
{
  if (glfwCreateWindowSurface(instance->get(), window, nullptr, &surface_) !=
      VK_SUCCESS)
    ThrowVk("Failed to create window surface!");
}

Surface::~Surface()
{
  if (surface_ != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(instance_->get(), surface_, nullptr);
}

} // namespace vk
} // namespace eldr
