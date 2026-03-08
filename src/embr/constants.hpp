#pragma once
#include "traits.hpp"
#include <eldr.hpp>

NAMESPACE_BEGIN(eldr::embr)
NAMESPACE_BEGIN(detail)

/// Constants template specialized for floating-point types
/// Uses hexadecimal float literals for exact representation
template <typename T> struct constants {
  // Mathematical constants
  static constexpr T kE = T(2.71828182845904523536);
  static constexpr T kLogTwo = T(0.69314718055994530942);
  static constexpr T kInvLogTwo = T(1.44269504088896340736);

  // Pi and related constants
  static constexpr T kPi = T(3.14159265358979323846);
  static constexpr T kInvPi = T(0.31830988618379067154);
  static constexpr T kSqrtPi = T(1.77245385090551602793);
  static constexpr T kInvSqrtPi = T(0.56418958354775628695);

  static constexpr T kTwoPi = T(6.28318530717958647692);
  static constexpr T kInvTwoPi = T(0.15915494309189533577);
  static constexpr T kSqrtTwoPi = T(2.50662827463100050242);
  static constexpr T kInvSqrtTwoPi = T(0.39894228040143267794);

  static constexpr T kFourPi = T(12.5663706143591729539);
  static constexpr T kInvFourPi = T(0.07957747154594766788);
  static constexpr T kSqrtFourPi = T(3.54490770181103205460);
  static constexpr T kInvSqrtFourPi = T(0.28209479177387814347);

  static constexpr T kSqrtTwo = T(1.41421356237309504880);
  static constexpr T kInvSqrtTwo = T(0.70710678118654752440);

  // Special floating-point values
#if defined(__GNUC__)
  static constexpr T kInfinity = T(__builtin_inf());
#else
  static constexpr T kInfinity = T(__builtin_huge_val());
#endif
  static constexpr T kNaN = T(__builtin_nan(""));

  /// Machine epsilon (smallest value where 1.0 + epsilon != 1.0)
  static constexpr T kEpsilon = T(sizeof(T) == 8 ? 0x1p-53   // double: 2^-53
                                                 : 0x1p-24); // float:  2^-24

  /// 1 - Machine epsilon
  static constexpr T kOneMinusEpsilon =
    T(sizeof(T) == 8 ? 0x1.fffffffffffffp-1 // double
                     : 0x1.fffffep-1);      // float

  /// Threshold below which reciprocal overflows to infinity
  static constexpr T kRecipOverflow = T(sizeof(T) == 8 ? 0x1p-1024  // double
                                                       : 0x1p-128); // float

  /// Smallest normalized floating point value
  static constexpr T kSmallest = T(sizeof(T) == 8 ? 0x1p-1022 // double: 2^-1022
                                                  : 0x1p-126); // float:  2^-126

  /// Largest normalized floating point value
  static constexpr T kLargest =
    T(sizeof(T) == 8 ? 0x1.fffffffffffffp+1023 // double
                     : 0x1.fffffep+127);       // float
};

NAMESPACE_END(detail)

// Public constants that extract scalar type and lookup in detail::constants
template <typename T> constexpr auto kE = detail::constants<scalar_t<T>>::kE;
template <typename T>
constexpr auto kLogTwo = detail::constants<scalar_t<T>>::kLogTwo;
template <typename T>
constexpr auto kInvLogTwo = detail::constants<scalar_t<T>>::kInvLogTwo;

template <typename T> constexpr auto kPi = detail::constants<scalar_t<T>>::kPi;
template <typename T>
constexpr auto kInvPi = detail::constants<scalar_t<T>>::kInvPi;
template <typename T>
constexpr auto kSqrtPi = detail::constants<scalar_t<T>>::kSqrtPi;
template <typename T>
constexpr auto kInvSqrtPi = detail::constants<scalar_t<T>>::kInvSqrtPi;

template <typename T>
constexpr auto kTwoPi = detail::constants<scalar_t<T>>::kTwoPi;
template <typename T>
constexpr auto kInvTwoPi = detail::constants<scalar_t<T>>::kInvTwoPi;
template <typename T>
constexpr auto kSqrtTwoPi = detail::constants<scalar_t<T>>::kSqrtTwoPi;
template <typename T>
constexpr auto kInvSqrtTwoPi = detail::constants<scalar_t<T>>::kInvSqrtTwoPi;

template <typename T>
constexpr auto kFourPi = detail::constants<scalar_t<T>>::kFourPi;
template <typename T>
constexpr auto kInvFourPi = detail::constants<scalar_t<T>>::kInvFourPi;
template <typename T>
constexpr auto kSqrtFourPi = detail::constants<scalar_t<T>>::kSqrtFourPi;
template <typename T>
constexpr auto kInvSqrtFourPi = detail::constants<scalar_t<T>>::kInvSqrtFourPi;

template <typename T>
constexpr auto kSqrtTwo = detail::constants<scalar_t<T>>::kSqrtTwo;
template <typename T>
constexpr auto kInvSqrtTwo = detail::constants<scalar_t<T>>::kInvSqrtTwo;

template <typename T>
constexpr auto kInfinity = detail::constants<scalar_t<T>>::kInfinity;
template <typename T>
constexpr auto kNaN = detail::constants<scalar_t<T>>::kNaN;

/// Machine epsilon
template <typename T>
constexpr auto kEpsilon = detail::constants<scalar_t<T>>::kEpsilon;
/// 1 - Machine epsilon
template <typename T>
constexpr auto kOneMinusEpsilon =
  detail::constants<scalar_t<T>>::kOneMinusEpsilon;

/// Threshold below which reciprocal overflows to infinity
template <typename T>
constexpr auto kRecipOverflow = detail::constants<scalar_t<T>>::kRecipOverflow;

/// Smallest normalized floating point value
template <typename T>
constexpr auto kSmallest = detail::constants<scalar_t<T>>::kSmallest;
/// Largest normalized floating point value
template <typename T>
constexpr auto kLargest = detail::constants<scalar_t<T>>::kLargest;

NAMESPACE_END(eldr::embr)
