#include <eldr/eldr.hpp>

#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main(int argc, char* argv[])
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

  // Run Eldr main app
  eldr::EldrApp main_app{};
  try {
    main_app.run();
  }
  catch (const std::exception& e) {
    spdlog::error("{}", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
