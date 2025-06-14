#include <eldr/app/app.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/core/util.hpp>

#include <cxxopts.hpp>

#include <iostream>

using namespace eldr;

NAMESPACE_BEGIN()
void help(std::string_view help)
{
  std::cout << util::infoBuild(1) << "\n";
  std::cout << util::infoCopyright() << "\n";
  std::cout << help << "\n";
}
NAMESPACE_END()

int main(int argc, char* argv[])
{
  Thread::createContext();
  Logger::createContext();

  cxxopts::Options options("Eldr", "Physically Based Renderer");
  // clang-format off
  options.add_options()
    ("t,threads",
    "Number of threads to render with. 0 will maximize performance.",
    cxxopts::value<int>()->default_value("0"))
    ("h,help", "Prints help.");
  // clang-format on

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    help(options.help({ "", "Group" }));
    return 0;
  }
  const int threads{ result["threads"].as<int>() };

  std::cout <<
    R"(###########################################
Running Eldr with the following settings:
    Number of threads: )"
            << threads << R"(
###########################################
)";

  // Run Eldr main app
  eldr::App main_app;
  try {
    main_app.run();
  }
  catch (const std::exception& e) {
    Log(eldr::Critical, "{}", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
