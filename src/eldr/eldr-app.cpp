#include <eldr/eldr.hpp>

namespace eldr {

// TODO: find a way to reduce window lag when moving etc. Limiting framerate
// kind of works but maybe there's a better solution
//constexpr int max_frame_rate    = 60;
//constexpr int min_frame_time_ms = 1000 / max_frame_rate;

void EldrApp::run()
{
  renderer_.submitGeometry(scene_.getShapes());
  while (renderer_.running()) {
    glfwPollEvents();
    renderer_.display();
  }
}

} // Namespace eldr
