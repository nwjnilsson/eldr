#include <eldr/app/window.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr::vk::wr)

EL_VK_IMPL_DEFAULTS(Surface)
Surface::~Surface()
{
  if (vk()) {
    vkDestroySurfaceKHR(instance_->vk(), object_, nullptr);
  }
}

Surface::Surface(std::string_view name,
                 const Instance&  instance,
                 const Window&    window)
  : Base(name), instance_(&instance)
{
  if (const VkResult result{ glfwCreateWindowSurface(
        instance.vk(), window.glfw(), nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create window surface! ({})", result);
}

NAMESPACE_END(eldr::vk::wr)
