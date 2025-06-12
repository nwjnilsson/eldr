#pragma once
#include <eldr/app/fwd.hpp>
#include <eldr/vulkan/vulkan.hpp>

NAMESPACE_BEGIN(eldr::vk::wr)

class Surface {
public:
  Surface();
  Surface(const Instance&, const app::Window&);
  ~Surface();

  Surface& operator=(Surface&&);

  [[nodiscard]] VkSurfaceKHR vk() const;

private:
  class SurfaceImpl;
  std::unique_ptr<SurfaceImpl> d_;
};
NAMESPACE_END(eldr::vk::wr)
