#include <eldr/app/window.hpp>
#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(eldr::vk::wr)

Surface ::Surface()                   = default;
Surface ::Surface(Surface&&) noexcept = default;
Surface& Surface::operator=(Surface&& o)
{
  if (this != &o) {
    if (vk()) {
      vkDestroySurfaceKHR(instance().vk(), object_, nullptr);
    }
    Base::operator=(std ::move(o));
  }
  return *this;
}

Surface::~Surface()
{
  if (vk()) {
    vkDestroySurfaceKHR(instance().vk(), object_, nullptr);
  }
}

Surface::Surface(std::string_view name,
                 const Instance&  instance,
                 const Window&    window)
  : Base(name, instance)
{
  if (const VkResult result{ glfwCreateWindowSurface(
        instance.vk(), window.glfw(), nullptr, &object_) };
      result != VK_SUCCESS)
    Throw("Failed to create window surface! ({})", result);
}

NAMESPACE_END(eldr::vk::wr)
