#pragma once

#include <cstring>

namespace eldr {
template <typename T, typename U> T memcpy_cast(const U& u)
{
  static_assert(sizeof(T) == sizeof(U), "memcpy_cast: sizes do not match!");
  T result;
  memcpy(&result, &u, sizeof(T));
  return result;
}
} // namespace eldr
