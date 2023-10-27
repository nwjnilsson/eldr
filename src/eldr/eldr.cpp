#include "eldr/eldr.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cxxopts.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace eldr {
void App::run()
{
  eldr_viewer_.init();
  while (!eldr_viewer_.should_close()) {
    glfwPollEvents();
  }

}

} // Namespace eldr

int main(int argc, char *argv[])
{
  cxxopts::Options options("Eldr", "Physically Based Renderer");
  options.add_options()(
      "t,threads",
      "Number of threads to render with. 0 will maximize performance.",
      cxxopts::value<int>()->default_value("0"))("h,help", "Prints help.");

  auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help({ "", "Group" }) << std::endl;
    return 0;
  }
  int threads = result["threads"].as<int>();

  std::cout <<
      R"(###########################################
Running Eldr with the following settings:
    Number of threads: )"
            << threads << R"(
###########################################
)";

  // Initialize
  eldr::App main_app{};
  try {
    main_app.run();
  }
  catch (const std::exception &e) {
    spdlog::error("An error occurred {}", e.what());
  }
  return EXIT_SUCCESS;
}
