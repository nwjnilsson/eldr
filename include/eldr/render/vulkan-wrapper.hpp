#pragma once

// This define will make the glfw3 include also include vulkan.h
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <memory>
#include <vector>

namespace eldr {
namespace render {

// Constants
constexpr uint8_t max_frames_in_flight = 2;

// fwd
struct VkData;
////////////////////////////////////////////////////////////////////////////////

struct VkWrapperInitInfo {
  GLFWwindow*               window;
  std::vector<const char*>& instance_extensions;
};

class VkWrapper {
public:
  VkWrapper();
  ~VkWrapper();

  // FUNCTIONS
  void init(VkWrapperInitInfo&);
  void drawFrame();

private:
  std::unique_ptr<VkData> vk_data_;
};

} // namespace render
} // namespace eldr
