#pragma once
#include <eldr/gui/gui.hpp>
#include <eldr/render/scene.hpp>

namespace eldr {

class EldrApp {
public:
  EldrApp(){};

  void run();

private:
  static constexpr int WIDTH  = 1280;
  static constexpr int HEIGHT = 720;
  // Initializing and cleaning up of GUI happens in constructor/destructor
  EldrGUI gui_{ WIDTH, HEIGHT, "Eldr" };
  Scene   scene_;
};

} // Namespace eldr
