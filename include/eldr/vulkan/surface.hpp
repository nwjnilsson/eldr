#pragma once

#include <eldr/vulkan/common.hpp>
#include <eldr/vulkan/instance.hpp>
#include <eldr/vulkan/surface.hpp>

namespace eldr {
namespace vk {

class Surface {
public:
  Surface(const Instance*, GLFWwindow*);
  ~Surface();

  const VkSurfaceKHR&       get() const { return surface_; }
  const VkSurfaceFormatKHR& format() const { return format_; }
  VkPresentModeKHR          presentMode() const { return present_mode_; }

  void setFormat(VkSurfaceFormatKHR new_format) { format_ = new_format; }

  void setPresentMode(VkPresentModeKHR new_present_mode)
  {
    present_mode_ = new_present_mode;
  }

private:
  const Instance* instance_;

  VkSurfaceKHR       surface_;
  VkSurfaceFormatKHR format_;
  VkPresentModeKHR   present_mode_;
};
} // namespace vk
} // namespace eldr
