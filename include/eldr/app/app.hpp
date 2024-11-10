#pragma once
#include <eldr/render/scene.hpp>

#include <memory>

// -----------------------------------------------------------------------------
// fwd
// -----------------------------------------------------------------------------
struct GLFWwindow;
namespace eldr {
class Window;
namespace vk {
class VulkanEngine;
}
// -----------------------------------------------------------------------------

class EldrApp {
  const std::string model_path_str   = "assets/models/viking_room.obj";
  const std::string texture_path_str = "assets/textures/viking_room.png";

public:
  EldrApp();
  ~EldrApp();

  void resize() const;
  void run();

private:
  void setupWindowCallbacks();
  void updateImGui();
  void submitGeometry(const std::vector<Shape*>&);

public:
  static constexpr uint32_t width  = 1280;
  static constexpr uint32_t height = 720;

private:
  std::unique_ptr<Window>           window_;
  std::unique_ptr<vk::VulkanEngine> vk_engine_;
  Scene                             scene_;
};

} // Namespace eldr
