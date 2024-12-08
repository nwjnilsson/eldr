#pragma once
#include <eldr/vulkan/common.hpp>

// fwd
struct GLFWwindow;

namespace eldr::vk::wr {

class Surface {
public:
  Surface() = default;
  Surface(const Instance&, GLFWwindow*);

  [[nodiscard]] VkSurfaceKHR get() const;

private:
  class SurfaceImpl;
  std::shared_ptr<SurfaceImpl> s_data_;
};
} // namespace eldr::vk::wr
