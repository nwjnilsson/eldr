#pragma once
#include "constants.hpp"
#include "traits.hpp"

NAMESPACE_BEGIN(eldr::embr)

// -----------------------------------------------------------------------------
// Spherical geometry utilities
// -----------------------------------------------------------------------------
// These are simple standalone functions, not routed operations.

/// Convert radians to degrees
template <typename T> EL_INLINE T radToDeg(const T& a)
{
  return a * scalar_t<T>(180.0 / kPi<scalar_t<T>>);
}

/// Convert degrees to radians
template <typename T> EL_INLINE T degToRad(const T& a)
{
  return a * scalar_t<T>(kPi<scalar_t<T>> / 180.0);
}

// Future additions from DrJit's sphere.h:
// - sphdir: Spherical coordinate parameterization
// - unit_angle: Numerically stable angle between vectors
// - unit_angle_z: Angle between vector and z-axis

NAMESPACE_END(eldr::embr)
