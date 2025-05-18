#include <eldr/app/app.hpp>

#include <eldr/core/logger.hpp>
#include <eldr/core/thread.hpp>
#include <eldr/core/util.hpp>

#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <iostream>

using namespace eldr::core;

namespace {
void help(std::string_view help)
{
  std::cout << util::infoBuild() << "\n";
  std::cout << util::infoCopyright() << "\n";
  std::cout << help << "\n";
}
} // namespace

int main(int argc, char* argv[])
{
  Thread::createContext();
  Logger::createContext();

  cxxopts::Options options("Eldr", "Physically Based Renderer");
  // clang-format off
  options.add_options()
    ("t,threads",
    "Number of threads to render with. 0 will maximize performance.",
    cxxopts::value<int>()->default_value(0))
    ("h,help", "Prints help.");
  // clang-format on

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help({ "", "Group" }) << "\n";
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

  spdlog::set_level((spdlog::level::level_enum) SPDLOG_ACTIVE_LEVEL);

  // Run Eldr main app
  eldr::EldrApp main_app;
  try {
    main_app.run();
  }
  catch (const std::exception& e) {
    spdlog::error("{}", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
