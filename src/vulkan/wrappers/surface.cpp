#include <eldr/vulkan/wrappers/instance.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

#include <GLFW/glfw3.h>
#include <eldr/app/window.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

//------------------------------------------------------------------------------
// SurfaceImpl
//------------------------------------------------------------------------------
class Surface::SurfaceImpl {
public:
  SurfaceImpl(const Instance& instance, const app::Window& window);
  ~SurfaceImpl();
  const Instance& instance_;
  VkSurfaceKHR    surface_{ VK_NULL_HANDLE };
};

Surface::SurfaceImpl::SurfaceImpl(const Instance&    instance,
                                  const app::Window& window)
  : instance_(instance)
{
  if (const VkResult result{ glfwCreateWindowSurface(
        instance.vk(), window.glfw(), nullptr, &surface_) };
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
Surface::Surface()                     = default;
Surface::~Surface()                    = default;
Surface& Surface::operator=(Surface&&) = default;

Surface::Surface(const Instance& instance, const app::Window& window)
  : d_(std::make_unique<SurfaceImpl>(instance, window))
{
}

VkSurfaceKHR Surface::vk() const { return d_->surface_; }
NAMESPACE_END(eldr::vk::wr)
