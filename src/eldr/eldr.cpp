#include <spdlog/spdlog.h>
#include <cxxopts.hpp>
#include <iostream>

int main (int argc, char *argv[])
{
    cxxopts::Options options("Eldr", "Physically Based Renderer");
    options.add_options()
        ("t,threads",
            "Number of threads to render with. 0 will maximize performance.",
            cxxopts::value<int>()->default_value("0"))
        ("h,help", "Prints help.");

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help({"", "Group"}) << std::endl;
        return 0;
    }
    int threads = result["threads"].as<int>();

    std::cout << 
R"(###########################################
Running Eldr with the following settings:
    Number of threads: )" << threads << R"(
###########################################
)";

    return 0;
}
