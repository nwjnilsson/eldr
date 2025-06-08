#pragma once
#include <filesystem>
#include <string>

namespace eldr::core::util {

int terminalWidth();

std::string infoCopyright();

std::string infoBuild(int thread_count);

std::filesystem::path eldrRootDir();
} // namespace eldr::core::util
