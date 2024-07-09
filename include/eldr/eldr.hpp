#pragma once
#include <eldr/render/renderer.hpp>
#include <eldr/render/scene.hpp>

namespace eldr {
class EldrApp {
  const std::string model_path_str   = "models/viking_room.obj";
  const std::string texture_path_str = "textures/viking_room.png";

public:
  inline EldrApp()
    : renderer_(width, height), scene_({ model_path_str, texture_path_str })
  {
  }

  void run();

  static constexpr uint32_t width  = 1280;
  static constexpr uint32_t height = 720;

private:
  Renderer renderer_;
  Scene    scene_;
};

} // Namespace eldr
