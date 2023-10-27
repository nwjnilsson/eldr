#pragma once

#include <gui/viewer.hpp>

namespace eldr {

class App {
  public:
    App(){};
    static constexpr int WIDTH  = 1280;
    static constexpr int HEIGHT = 720;

    void run();

  private:
    Viewer eldr_viewer_{ WIDTH, HEIGHT, "Eldr" };
};

} // Namespace eldr
