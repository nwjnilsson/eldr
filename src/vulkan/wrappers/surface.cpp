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
  if (const auto result =
        glfwCreateWindowSurface(instance.get(), window, nullptr, &surface_);
      result != VK_SUCCESS)
    ThrowVk(result, "glfwCreateWindowSurface(): ");
}

Surface::SurfaceImpl::~SurfaceImpl()
{
  vkDestroySurfaceKHR(instance_.get(), surface_, nullptr);
}

//------------------------------------------------------------------------------
// Surface
//------------------------------------------------------------------------------
Surface::Surface(const Instance& instance, GLFWwindow* window)
  : s_data_(std::make_shared<SurfaceImpl>(instance, window))
{
}

VkSurfaceKHR Surface::get() const { return s_data_->surface_; }
} // namespace eldr::vk::wr
