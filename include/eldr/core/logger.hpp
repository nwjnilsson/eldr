#pragma once

#include <spdlog/spdlog.h>
#include <stdexcept>

#define Throw(...)                                                             \
  do {                                                                         \
    throw std::runtime_error(fmt::format(__VA_ARGS__));                        \
  } while (0)
