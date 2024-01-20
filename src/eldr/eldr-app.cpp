#include <eldr/eldr.hpp>
#include <spdlog/spdlog.h>

namespace eldr {


void EldrApp::run()
{
  gui_.init();
  while (!gui_.shouldClose()) {
    glfwPollEvents();
    scene_.update();
    gui_.display();
  }
  gui_.terminate();
}

} // Namespace eldr
