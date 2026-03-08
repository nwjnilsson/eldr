#pragma once
#include "arrayrouter.hpp"
#include <cmath>

NAMESPACE_BEGIN(eldr::embr)

// -----------------------------------------------------------------------------
// Transcendental functions
// Scalar: forward to std::. Array: recurse over entries.
//
// Fundamental operations (sqrt, abs, floor, ceil, round, trunc) are routed
// through member functions in arraybase.hpp for SIMD override. Transcendentals
// here are standalone free functions with no corresponding members.
//
// When CEPHES polynomial approximations are added (requiring select(),
// reinterpret_array, int_array_t, estrin(), etc.), these can be replaced
// with DrJit-style implementations from drjit/math.h.
// -----------------------------------------------------------------------------

// Unary transcendentals
#define EL_MATH_UNARY(name, std_func)                                          \
  template <typename T> EL_INLINE T name(const T& a)                           \
  {                                                                            \
    if constexpr (!is_array_v<T>) {                                            \
      return std_func(a);                                                      \
    }                                                                          \
    else {                                                                     \
      T result;                                                                \
      for (size_t i = 0; i < size_v<T>; ++i)                                   \
        result.entry(i) = name(a.entry(i));                                    \
      return result;                                                           \
    }                                                                          \
  }

EL_MATH_UNARY(sin, std::sin)
EL_MATH_UNARY(cos, std::cos)
EL_MATH_UNARY(tan, std::tan)
EL_MATH_UNARY(asin, std::asin)
EL_MATH_UNARY(acos, std::acos)
EL_MATH_UNARY(atan, std::atan)
EL_MATH_UNARY(exp, std::exp)
EL_MATH_UNARY(exp2, std::exp2)
EL_MATH_UNARY(log, std::log)
EL_MATH_UNARY(log2, std::log2)
EL_MATH_UNARY(sinh, std::sinh)
EL_MATH_UNARY(cosh, std::cosh)
EL_MATH_UNARY(tanh, std::tanh)
EL_MATH_UNARY(asinh, std::asinh)
EL_MATH_UNARY(acosh, std::acosh)
EL_MATH_UNARY(atanh, std::atanh)
EL_MATH_UNARY(cbrt, std::cbrt)
EL_MATH_UNARY(erf, std::erf)

#undef EL_MATH_UNARY

// Binary transcendentals
#define EL_MATH_BINARY(name, std_func)                                         \
  template <typename T1, typename T2>                                          \
  EL_INLINE expr_t<T1, T2> name(const T1& a1, const T2& a2)                   \
  {                                                                            \
    using E = expr_t<T1, T2>;                                                  \
    if constexpr (!is_array_any_v<T1, T2>) {                                   \
      return std_func(a1, a2);                                                 \
    }                                                                          \
    else if constexpr (std::is_same_v<T1, E> && std::is_same_v<T2, E>) {      \
      E result;                                                                \
      for (size_t i = 0; i < size_v<E>; ++i)                                   \
        result.entry(i) = name(a1.entry(i), a2.entry(i));                      \
      return result;                                                           \
    }                                                                          \
    else {                                                                     \
      return name(static_cast<ref_cast_t<T1, E>>(a1),                          \
                  static_cast<ref_cast_t<T2, E>>(a2));                         \
    }                                                                          \
  }

EL_MATH_BINARY(atan2, std::atan2)
EL_MATH_BINARY(pow, std::pow)

#undef EL_MATH_BINARY

// -----------------------------------------------------------------------------
// Simultaneous sine and cosine
// DrJit implements this with CEPHES (drjit/math.h:76-185). For now, we
// use std::sin/cos for scalars and recurse for arrays. The interface is
// ready for a CEPHES replacement when prerequisites are in place.
// -----------------------------------------------------------------------------

/// Compute sine and cosine simultaneously. Returns {sin, cos}.
template <typename T> EL_INLINE std::pair<T, T> sincos(const T& a)
{
  if constexpr (!is_array_v<T>) {
    return { std::sin(a), std::cos(a) };
  }
  else {
    T s, c;
    for (size_t i = 0; i < size_v<T>; ++i) {
      auto [si, ci] = sincos(a.entry(i));
      s.entry(i) = si;
      c.entry(i) = ci;
    }
    return { s, c };
  }
}

/// Simultaneous hyperbolic sine and cosine
template <typename T> EL_INLINE std::pair<T, T> sincosh(const T& a)
{
  if constexpr (!is_array_v<T>) {
    return { std::sinh(a), std::cosh(a) };
  }
  else {
    T s, c;
    for (size_t i = 0; i < size_v<T>; ++i) {
      auto [si, ci] = sincosh(a.entry(i));
      s.entry(i) = si;
      c.entry(i) = ci;
    }
    return { s, c };
  }
}

NAMESPACE_END(eldr::embr)
