#pragma once

#include <stdexcept>

#define Throw(...)                                                             \
  do {                                                                         \
    throw std::runtime_error(fmt::format("[File: {}, line: {}] {}",            \
                                         __FILE__,                             \
                                         __LINE__,                             \
                                         fmt::format(__VA_ARGS__)));           \
  } while (0)

