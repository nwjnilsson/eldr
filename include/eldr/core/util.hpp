#pragma once
#include <string>

namespace eldr::core::util {

int terminalWidth();

std::string infoCopyright();

std::string infoBuild(int thread_count);

} // namespace eldr::core::util
