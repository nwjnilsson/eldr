#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <GLFW/glfw3.h>

namespace eldr::vk::wr {

//------------------------------------------------------------------------------
// SurfaceImpl
//------------------------------------------------------------------------------
class Surface::SurfaceImpl {
public:
  SurfaceImpl(const Instance& instance, GLFWwindow* window);
  ~SurfaceImpl();
  const Instance instance_;
  VkSurfaceKHR   surface_{ VK_NULL_HANDLE };
};

Surface::SurfaceImpl::SurfaceImpl(const Instance& instance, GLFWwindow* window)
  : instance_(instance)
{
  if (const VkResult result{
        glfwCreateWindowSurface(instance.vk(), window, nullptr, &surface_) };
      result != VK_SUCCESS)
    Throw("Failed to create window surface! ({})", result);
}

Surface::SurfaceImpl::~SurfaceImpl()
{
  vkDestroySurfaceKHR(instance_.vk(), surface_, nullptr);
}

//------------------------------------------------------------------------------
// Surface
//------------------------------------------------------------------------------
Surface::Surface(const Instance& instance, GLFWwindow* window)
  : s_data_(std::make_shared<SurfaceImpl>(instance, window))
{
}

VkSurfaceKHR Surface::vk() const { return s_data_->surface_; }
} // namespace eldr::vk::wr
