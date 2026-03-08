#pragma once
#include "matrix.hpp"

#include "quaternion.hpp"

NAMESPACE_BEGIN(eldr::embr)

// -----------------------------------------------------------------------------
// Translation matrix
// -----------------------------------------------------------------------------

/// Create a translation matrix from a (Size-1)-dimensional vector
template <typename Mat, typename T>
  requires(is_matrix_v<Mat> && is_array_v<T> && size_v<T> == size_v<Mat> - 1)
EL_INLINE Mat translate(const T& v)
{
  constexpr size_t N = size_v<Mat>;
  Mat              result = Mat::identity();
  for (size_t i = 0; i < N - 1; ++i)
    result(i, N - 1) = v.entry(i);
  return result;
}

// -----------------------------------------------------------------------------
// Scale matrix
// -----------------------------------------------------------------------------

/// Create a scale matrix from a (Size-1)-dimensional vector
template <typename Mat, typename T>
  requires(is_matrix_v<Mat> && is_array_v<T> && size_v<T> == size_v<Mat> - 1)
EL_INLINE Mat scale(const T& v)
{
  return diag(concat(v, Array<value_t<T>, 1>(value_t<T>(1))));
}

// -----------------------------------------------------------------------------
// Rotation matrices
// -----------------------------------------------------------------------------

/// Create a 2D rotation matrix (3x3). Angle in radians.
template <typename Mat>
  requires(is_matrix_v<Mat> && size_v<Mat> == 3)
EL_INLINE Mat rotate(const typename Mat::Entry& angle)
{
  using T = typename Mat::Entry;
  T z(0), o(1);
  auto [s, c] = sincos(angle);
  return Mat(c, -s, z, s, c, z, z, z, o);
}

/// Create a 3D rotation matrix (4x4) from axis and angle. Angle in radians.
/// Accepts any 3D array type for axis (Array, Vector, etc.)
template <typename Mat, typename Vec>
  requires(is_matrix_v<Mat> && size_v<Mat> == 4 && is_array_v<Vec> &&
           size_v<Vec> == 3)
EL_INLINE Mat rotate(const Vec& axis, const typename Mat::Entry& angle)
{
  using T = typename Mat::Entry;
  using Vec3 = Array<T, 3>;
  using Vec4 = Array<T, 4>;

  auto [sin_theta, cos_theta] = sincos(angle);
  T cos_theta_m = T(1) - cos_theta;

  // Construct axis as Vec3 for shuffle operations
  Vec3 a(axis.entry(0), axis.entry(1), axis.entry(2));

  Vec3 shuf1 = shuffle<1, 2, 0>(a), shuf2 = shuffle<2, 0, 1>(a),
       tmp0 = fmadd(a * a, Vec3(cos_theta_m), Vec3(cos_theta)),
       tmp1 = fmadd(a * shuf1, Vec3(cos_theta_m), shuf2 * sin_theta),
       tmp2 = fmsub(a * shuf2, Vec3(cos_theta_m), shuf1 * sin_theta);

  return Mat(Vec4(tmp0.x(), tmp2.y(), tmp1.z(), T(0)),
             Vec4(tmp1.x(), tmp0.y(), tmp2.z(), T(0)),
             Vec4(tmp2.x(), tmp1.y(), tmp0.z(), T(0)),
             Vec4(T(0), T(0), T(0), T(1)));
}

template <typename Mat4, typename Vec>
  requires(is_matrix_v<Mat4> && size_v<Mat4> == 4 && is_array_v<Vec> &&
           size_v<Vec> == 3)
EL_INLINE Mat4 transformCompose(const Matrix<entry_t<Mat4>, 3>&  s,
                                const Quaternion<entry_t<Mat4>>& q,
                                const Vec&                       t)
{
  using Value = entry_t<Mat4>;

  Mat4 result(quatToMatrix<Matrix<Value, 3>>(q) * s);
  for (size_t i = 0; i < 3; ++i)
    result(i, 3) = t.entry(i);
  result(3, 3) = 1;

  return result;
}

NAMESPACE_END(eldr::embr)
