#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/vulkan/vulkan.hpp>

namespace eldr::vk::wr {

class Surface {
public:
  Surface() = default;
  Surface(const Instance&, const app::Window&);

  [[nodiscard]] VkSurfaceKHR vk() const;

private:
  class SurfaceImpl;
  std::shared_ptr<SurfaceImpl> d_;
};
} // namespace eldr::vk::wr
