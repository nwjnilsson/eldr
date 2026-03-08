#pragma once
#include "arrayiface.hpp"
#include "math.hpp"

NAMESPACE_BEGIN(eldr::embr)

/// Quaternion type: (x, y, z) imaginary + w real component
/// Stored as [x, y, z, w] (imaginary first, real last)
template <typename Value_>
struct Quaternion : StaticArray<Value_, 4, false, Quaternion<Value_>> {
  using Base = StaticArray<Value_, 4, false, Quaternion<Value_>>;
  EL_ARRAY_DEFAULTS(Quaternion)

  static constexpr bool kIsQuaternion = true;
  static constexpr bool kIsSpecial = true;

  template <typename T> using ReplaceValue = Quaternion<T>;

  Quaternion() = default;

  /// Construct from 4 components (x, y, z imaginary, w real)
  Quaternion(const Value_& xi,
             const Value_& yj,
             const Value_& zk,
             const Value_& w)
    : Base(xi, yj, zk, w)
  {
  }

  /// Construct from imaginary (3D vector) and real part
  template <typename Im, typename Re>
    requires(is_array_v<Im> && size_v<Im> == 3)
  Quaternion(const Im& im, const Re& re)
    : Base(im.x(), im.y(), im.z(), Value_(re))
  {
  }

  /// Construct a pure-real quaternion from a scalar
  template <typename T>
    requires(!is_array_v<T> && std::is_convertible_v<T, Value_>)
  explicit Quaternion(T v) : Base(Value_(0), Value_(0), Value_(0), Value_(v))
  {
  }
};

// ---------------------------------------------------------------------------
// Quaternion identity
// ---------------------------------------------------------------------------

/// Return the quaternion identity (0, 0, 0, 1)
template <typename T> Quaternion<T> quatIdentity()
{
  return Quaternion<T>(T(0), T(0), T(0), T(1));
}

// ---------------------------------------------------------------------------
// Component access
// ---------------------------------------------------------------------------

/// Return the real (w) component
template <typename T> T real(const Quaternion<T>& q) { return q.entry(3); }

/// Return the imaginary (x, y, z) components
template <typename T> Array<T, 3> imag(const Quaternion<T>& q)
{
  return Array<T, 3>(q.entry(0), q.entry(1), q.entry(2));
}

// ---------------------------------------------------------------------------
// Conjugate
// ---------------------------------------------------------------------------

/// Quaternion conjugate: negate imaginary part
template <typename T> Quaternion<T> conj(const Quaternion<T>& q)
{
  return Quaternion<T>(-q.entry(0), -q.entry(1), -q.entry(2), q.entry(3));
}

// ---------------------------------------------------------------------------
// Quaternion multiplication (Hamilton product)
// ---------------------------------------------------------------------------

template <typename T0, typename T1>
Quaternion<expr_t<T0, T1>> operator*(const Quaternion<T0>& q0,
                                     const Quaternion<T1>& q1)
{
  using Value = expr_t<T0, T1>;
  using Base = Array<Value, 4>;
  Base a0 = Base(q0.entry(0), q0.entry(1), q0.entry(2), q0.entry(3));
  Base a1 = Base(q1.entry(0), q1.entry(1), q1.entry(2), q1.entry(3));

  Base t1 = fmadd(shuffle<0, 1, 2, 0>(a0),
                  shuffle<3, 3, 3, 0>(a1),
                  shuffle<1, 2, 0, 1>(a0) * shuffle<2, 0, 1, 1>(a1));
  Base t2 = fmsub(shuffle<3, 3, 3, 3>(a0),
                  a1,
                  shuffle<2, 0, 1, 2>(a0) * shuffle<1, 2, 0, 2>(a1));

  // Negate w component of t1
  t1.entry(3) = -t1.entry(3);

  Base r = t1 + t2;
  return Quaternion<Value>(r.entry(0), r.entry(1), r.entry(2), r.entry(3));
}

/// Scalar-quaternion multiplication
template <typename T0, typename T1>
  requires(!is_array_v<T0>)
Quaternion<expr_t<T0, T1>> operator*(const T0& s, const Quaternion<T1>& q)
{
  using V = expr_t<T0, T1>;
  return Quaternion<V>(
    V(s * q.entry(0)), V(s * q.entry(1)), V(s * q.entry(2)), V(s * q.entry(3)));
}

template <typename T0, typename T1>
  requires(!is_array_v<T1>)
Quaternion<expr_t<T0, T1>> operator*(const Quaternion<T0>& q, const T1& s)
{
  return s * q;
}

// ---------------------------------------------------------------------------
// Dot product and norm
// ---------------------------------------------------------------------------

template <typename T0, typename T1>
expr_t<T0, T1> dot(const Quaternion<T0>& q0, const Quaternion<T1>& q1)
{
  using V = expr_t<T0, T1>;
  return V(q0.entry(0)) * V(q1.entry(0)) + V(q0.entry(1)) * V(q1.entry(1)) +
         V(q0.entry(2)) * V(q1.entry(2)) + V(q0.entry(3)) * V(q1.entry(3));
}

template <typename T> T squaredNorm(const Quaternion<T>& q)
{
  return dot(q, q);
}

template <typename T> T norm(const Quaternion<T>& q)
{
  return sqrt(squaredNorm(q));
}

template <typename T> Quaternion<T> normalize(const Quaternion<T>& q)
{
  T inv_n = rsqrt(squaredNorm(q));
  return inv_n * q;
}

// ---------------------------------------------------------------------------
// Create quaternion from axis-angle rotation
// ---------------------------------------------------------------------------

/// Create a unit quaternion representing a rotation around axis by angle
/// (radians). Accepts any 3D array type (Array, Vector, etc.)
template <typename Vec, typename T = value_t<Vec>>
  requires(is_array_v<Vec> && size_v<Vec> == 3)
Quaternion<T> rotate(const Vec& axis, const T& angle)
{
  auto [s, c] = sincos(angle * T(0.5));
  return Quaternion<T>(axis * s, c);
}

// ---------------------------------------------------------------------------
// Convert quaternion to rotation matrix
// ---------------------------------------------------------------------------

/// Convert a unit quaternion to a rotation matrix (3x3 or 4x4)
template <typename Mat, typename T>
  requires(is_matrix_v<Mat> && (size_v<Mat> == 3 || size_v<Mat> == 4))
Mat quatToMatrix(const Quaternion<T>& q_)
{
  // Scale by sqrt(2) so the formulas use q*q directly (avoids factor of 2)
  constexpr T   kSqrtTwo = T(1.41421356237309504880);
  Quaternion<T> q = kSqrtTwo * q_;

  T xx = q.entry(0) * q.entry(0), yy = q.entry(1) * q.entry(1),
    zz = q.entry(2) * q.entry(2);
  T xy = q.entry(0) * q.entry(1), xz = q.entry(0) * q.entry(2),
    yz = q.entry(1) * q.entry(2);
  T xw = q.entry(0) * q.entry(3), yw = q.entry(1) * q.entry(3),
    zw = q.entry(2) * q.entry(3);

  if constexpr (size_v<Mat> == 4) {
    return Mat(T(1) - (yy + zz),
               xy - zw,
               xz + yw,
               T(0),
               xy + zw,
               T(1) - (xx + zz),
               yz - xw,
               T(0),
               xz - yw,
               yz + xw,
               T(1) - (xx + yy),
               T(0),
               T(0),
               T(0),
               T(0),
               T(1));
  }
  else {
    return Mat(T(1) - (yy + zz),
               xy - zw,
               xz + yw,
               xy + zw,
               T(1) - (xx + zz),
               yz - xw,
               xz - yw,
               yz + xw,
               T(1) - (xx + yy));
  }
}

NAMESPACE_END(eldr::embr)
