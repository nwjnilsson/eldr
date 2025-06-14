#pragma once
#include <eldr/eldr.hpp>
#include <filesystem>
#include <string>

NAMESPACE_BEGIN(eldr::util)

int terminalWidth();

std::string infoCopyright();

std::string infoBuild(int thread_count);

std::filesystem::path eldrRootDir();
NAMESPACE_END(eldr::util)
