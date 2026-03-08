#pragma once
#include <cmath>
#include <cstdlib>
#include <type_traits>

#if defined(_MSC_VER)
#  include <stdio.h>
#  define embr_fail(...)                                                       \
    do {                                                                       \
      printf(__VA_ARGS__);                                                     \
      abort();                                                                 \
    } while (0)
#else
#  define embr_fail(...)                                                       \
    do {                                                                       \
      __builtin_printf(__VA_ARGS__);                                           \
      abort();                                                                 \
    } while (0)
#endif

namespace eldr::embr::detail {

// -----------------------------------------------------------------------------
// Scalar fallback implementations for basic math operations
// These are used when the router dispatches to non-array types
// -----------------------------------------------------------------------------

template <typename T> T sqrt_(const T& a) { return std::sqrt(a); }

template <typename T> T rcp_(const T& a) { return T(1) / a; }

template <typename T> T rsqrt_(const T& a) { return rcp_(std::sqrt(a)); }

template <typename T> T fmadd_(const T& a, const T& b, const T& c)
{
  if constexpr (std::is_floating_point_v<T>)
    return std::fma(a, b, c);
  else
    return a * b + c;
}

template <typename T> T abs_(const T& a) { return std::abs(a); }

template <typename T> T floor_(const T& a) { return std::floor(a); }

template <typename T> T ceil_(const T& a) { return std::ceil(a); }

template <typename T> T round_(const T& a) { return std::round(a); }

template <typename T> T trunc_(const T& a) { return std::trunc(a); }

template <typename T> T minimum_(const T& a, const T& b)
{
  return b < a ? b : a;
}

template <typename T> T maximum_(const T& a, const T& b)
{
  return a < b ? b : a;
}

} // namespace eldr::embr::detail
