#pragma once

#include "vector.hpp"

#include "embr/transform.hpp"

#include "fwd.hpp"

NAMESPACE_BEGIN(eldr)
/// Homogeneous coordinate transformation.
///
/// Stores both the matrix and its inverse transpose for efficient
/// transformation of normals. When Affine is true, the last row is assumed
/// to be [0 0 ... 0 1], enabling optimized operations.
template <typename _Point, bool _Affine> struct Transform {
  using Float = em::value_t<_Point>;
  using Matrix = em::Matrix<Float, _Point::kSize>;

  EL_IMPORT_CORE_TYPES();
  static constexpr size_t kSize = _Point::kSize;
  static constexpr bool   kIsAffine = _Affine;

  Matrix matrix = Matrix::identity();
  Matrix inverse_transpose = Matrix::identity();

  Transform() = default;

  /// Initialize from a matrix (computes inverse transpose automatically)
  explicit Transform(const Matrix& m) : matrix(m) { update(); }

  /// Initialize from a matrix and its inverse transpose
  Transform(const Matrix& m, const Matrix& inv_t)
    : matrix(m), inverse_transpose(inv_t)
  {
  }

  /// Converting constructor between affine/projective transforms
  template <typename OtherPoint, bool OtherAffine>
  Transform(const Transform<OtherPoint, OtherAffine>& other)
    : matrix(Matrix(other.matrix)),
      inverse_transpose(Matrix(other.inverse_transpose))
  {
  }

  /// Return the inverse of this transform
  Transform inverse() const
  {
    return Transform(em::transpose(inverse_transpose), em::transpose(matrix));
  }

  /// Return the transpose of this transform
  Transform transpose() const
  {
    return Transform(em::transpose(matrix), em::transpose(inverse_transpose));
  }

  /// Get the translation component
  Vector<Float, kSize - 1> translation() const
  {
    Vector<Float, kSize - 1> result;
    for (size_t i = 0; i < kSize - 1; ++i)
      result.entry(i) = matrix(i, kSize - 1);
    return result;
  }

  /// Recompute inverse_transpose from matrix
  void update()
  {
    if constexpr (_Affine) {
      using RotMatrix = em::Matrix<Float, kSize - 1>;

      // Extract the rotation/scale part
      RotMatrix rot;
      for (size_t i = 0; i < kSize - 1; ++i)
        for (size_t j = 0; j < kSize - 1; ++j)
          rot(i, j) = matrix(i, j);

      RotMatrix invt_rot = em::inverse_transpose(rot);

      // Compute inverse translation: -R^{-1} * t
      // inv_trans = -transpose(invt_rot) * translation
      auto trans = translation();
      auto inv_rot = em::transpose(invt_rot);

      inverse_transpose = Matrix::identity();
      for (size_t i = 0; i < kSize - 1; ++i) {
        for (size_t j = 0; j < kSize - 1; ++j)
          inverse_transpose(i, j) = invt_rot(i, j);
      }

      // Last column of inverse = -R^{-1} * t, stored in last row of inv_t
      for (size_t i = 0; i < kSize - 1; ++i) {
        Float sum(0);
        for (size_t k = 0; k < kSize - 1; ++k)
          sum += inv_rot(i, k) * trans.entry(k);
        inverse_transpose(kSize - 1, i) = -sum;
      }
    }
    else {
      inverse_transpose = em::inverse_transpose(matrix);
    }
  }

  /// Compose transforms
  Transform operator*(const Transform& o) const
  {
    if constexpr (_Affine) {
      Transform result;

      // Upper-left (kSize-1)x(kSize-1) block
      for (size_t i = 0; i < kSize - 1; ++i) {
        for (size_t j = 0; j < kSize - 1; ++j) {
          Float sum(0), sum_it(0);
          for (size_t k = 0; k < kSize - 1; ++k) {
            sum += matrix(i, k) * o.matrix(k, j);
            sum_it += inverse_transpose(i, k) * o.inverse_transpose(k, j);
          }
          result.matrix(i, j) = sum;
          result.inverse_transpose(i, j) = sum_it;
        }
      }

      // Last column / last row
      for (size_t l = 0; l < kSize - 1; ++l) {
        Float sum = matrix(l, kSize - 1);
        Float sum_it = o.inverse_transpose(kSize - 1, l);

        for (size_t k = 0; k < kSize - 1; ++k) {
          sum += matrix(l, k) * o.matrix(k, kSize - 1);
          sum_it += inverse_transpose(kSize - 1, k) * o.inverse_transpose(k, l);
        }

        result.matrix(l, kSize - 1) = sum;
        result.inverse_transpose(kSize - 1, l) = sum_it;
      }

      return result;
    }
    else {
      return Transform(matrix * o.matrix,
                       inverse_transpose * o.inverse_transpose);
    }
  }

  /// Transform a vector (direction only, ignores translation)
  template <typename T>
  Vector<T, kSize - 1> operator*(const Vector<T, kSize - 1>& v) const
  {
    Vector<T, kSize - 1> result;
    for (size_t i = 0; i < kSize - 1; ++i)
      result.entry(i) = matrix(i, 0) * v.entry(0);
    for (size_t j = 1; j < kSize - 1; ++j)
      for (size_t i = 0; i < kSize - 1; ++i)
        result.entry(i) += matrix(i, j) * v.entry(j);
    return result;
  }

  /// Transform a normal (uses inverse transpose)
  template <typename T>
  Normal<T, kSize - 1> operator*(const Normal<T, kSize - 1>& n) const
  {
    Normal<T, kSize - 1> result;
    for (size_t i = 0; i < kSize - 1; ++i)
      result.entry(i) = inverse_transpose(i, 0) * n.entry(0);
    for (size_t j = 1; j < kSize - 1; ++j)
      for (size_t i = 0; i < kSize - 1; ++i)
        result.entry(i) += inverse_transpose(i, j) * n.entry(j);
    return result;
  }

  /// Transform a point (applies translation)
  template <typename T>
  Point<T, kSize - 1> operator*(const Point<T, kSize - 1>& p) const
  {
    if constexpr (_Affine) {
      Point<T, kSize - 1> result;
      for (size_t i = 0; i < kSize - 1; ++i)
        result.entry(i) = matrix(i, kSize - 1);
      for (size_t j = 0; j < kSize - 1; ++j)
        for (size_t i = 0; i < kSize - 1; ++i)
          result.entry(i) += matrix(i, j) * p.entry(j);
      return result;
    }
    else {
      // Projective case with perspective division
      em::Array<T, kSize> h;
      for (size_t i = 0; i < kSize; ++i)
        h.entry(i) = matrix(i, kSize - 1);
      for (size_t j = 0; j < kSize - 1; ++j)
        for (size_t i = 0; i < kSize; ++i)
          h.entry(i) += matrix(i, j) * p.entry(j);

      Point<T, kSize - 1> result;
      T                   inv_w = T(1) / h.entry(kSize - 1);
      for (size_t i = 0; i < kSize - 1; ++i)
        result.entry(i) = h.entry(i) * inv_w;
      return result;
    }
  }

  bool operator==(const Transform& o) const { return matrix == o.matrix; }
  bool operator!=(const Transform& o) const { return !(matrix == o.matrix); }

  // ---------------------------------------------------------------------------
  // Static factory methods
  // ---------------------------------------------------------------------------

  /// Create a translation transform
  static Transform translate(const Vector<Float, kSize - 1>& v)
  {
    return Transform(em::translate<Matrix>(v),
                     em::transpose(em::translate<Matrix>(-v)));
  }

  /// Create a scale transform
  static Transform scale(const Vector<Float, kSize - 1>& v)
  {
    return Transform(em::scale<Matrix>(v), em::scale<Matrix>(em::rcp(v)));
  }

  /// Create a rotation transform around an axis (angle in radians)
  /// Only available for 4x4 transforms
  template <size_t N = kSize>
    requires(N == 4)
  static Transform rotate(const Vector<Float, 3>& axis, Float angle)
  {
    Matrix m = em::rotate<Matrix>(axis, angle);
    return Transform(m, m); // Rotation: inverse_transpose == matrix
  }

  /// Create a look-at camera transformation
  template <size_t N = kSize>
    requires(N == 4)
  static Transform lookAt(const Point<Float, 3>&  origin,
                          const Point<Float, 3>&  target,
                          const Vector<Float, 3>& up)
  {
    using Vec3 = Vector<Float, 3>;
    using Vec4 = Vector<Float, 4>;

    Vec3 dir = em::normalize(Vec3(target - origin));
    Vec3 left = em::normalize(em::cross(up, dir));
    Vec3 new_up = em::cross(dir, left);

    em::Array<Float, 1> z(Float(0));
    Matrix              result = em::transpose(
      Matrix(Vec4(em::concat(left, z)),
             Vec4(em::concat(new_up, z)),
             Vec4(em::concat(dir, z)),
             Vec4(em::concat(Vec3(origin), em::Array<Float, 1>(Float(1))))));

    Matrix inv =
      em::transpose(Matrix(Vec4(em::concat(left, z)),
                           Vec4(em::concat(new_up, z)),
                           Vec4(em::concat(dir, z)),
                           Vec4(Float(0), Float(0), Float(0), Float(1))));

    inv[3] = em::transpose(inv) *
             Vec4(em::concat(-Vec3(origin), em::Array<Float, 1>(Float(1))));

    return Transform(result, inv);
  }

  /// Create a perspective transformation (maps [near, far] to [0, 1])
  /// Only available for 4x4 projective transforms
  template <size_t N = kSize, bool A = _Affine>
    requires(N == 4 && !A)
  static Transform perspective(Float fov, Float aspect, Float near, Float far)
  {
    Float recip = Float(1) / (far - near);
    Float tan = std::tan(fov * Float(0.5));
    Float cot = Float(1) / tan;

    Matrix trafo =
      em::diag(Vector<Float, 4>(cot / aspect, cot, far * recip, Float(0)));
    trafo(2, 3) = -near * far * recip;
    trafo(3, 2) = Float(1);

    Matrix inv = em::diag(
      Vector<Float, 4>(aspect * tan, tan, Float(0), Float(1) / near));
    inv(2, 3) = Float(1);
    inv(3, 2) = (near - far) / (far * near);

    return Transform(trafo, em::transpose(inv));
  }
};
NAMESPACE_END(eldr)
