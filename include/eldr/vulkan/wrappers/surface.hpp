#pragma once

#include <eldr/vulkan/common.hpp>

// fwd
struct GLFWwindow;

namespace eldr::vk::wr {

class Surface {
public:
  Surface(const Instance&, GLFWwindow*);
  ~Surface();

  VkSurfaceKHR get() const { return surface_; }

private:
  const Instance& instance_;
  VkSurfaceKHR    surface_{ VK_NULL_HANDLE };
};
} // namespace eldr::vk::wr
