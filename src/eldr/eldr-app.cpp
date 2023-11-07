#include <eldr/eldr.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace eldr {


void EldrApp::run()
{
  gui_.init();
  while (!gui_.shouldClose()) {
    glfwPollEvents();
  }
 
}

} // Namespace eldr
