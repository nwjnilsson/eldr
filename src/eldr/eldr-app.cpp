#include <eldr/eldr.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace eldr {


void EldrApp::run()
{
  while (!gui_.shouldClose()) {
    glfwPollEvents();
  }
 
}

} // Namespace eldr
