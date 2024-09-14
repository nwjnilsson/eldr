#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/wrappers/surface.hpp>

// fwd
struct GLFWwindow;

namespace eldr::vk::wr {

class Surface {
public:
  Surface(Instance&, GLFWwindow*);
  ~Surface();

  VkSurfaceKHR              get() const { return surface_; }

private:
  Instance&          instance_;
  VkSurfaceKHR       surface_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
