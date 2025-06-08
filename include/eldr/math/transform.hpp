#pragma once

#include <eldr/math/matrix.hpp>

namespace eldr::em {
template <typename Point_> struct Transform {
  static constexpr size_t Size{ Point_::Size };

  using Float  = em::value_t<Point_>::Scalar;
  using Matrix = em::Matrix<Float, Size>;

  Matrix matrix{ Matrix::identity() };

  Transform(const Matrix& m) : matrix(m) {}
  template <typename T>
    requires(not std::is_same_v<T, Point_>)
  Transform(const Transform<T>& transform) : matrix(transform.matrix)
  {
  }

  EL_INLINE Float&       operator()(size_t i, size_t j) { return matrix[i][j]; }
  EL_INLINE const Float& operator()(size_t i, size_t j) const
  {
    return matrix[i][j];
  }

  EL_INLINE Transform operator*(const Transform& other) const
  {
    return Transform{ matrix * other.matrix };
  }

  template <size_t S = Size>
    requires(S == 4)
  static Transform lookAt(const Point<Float, 3>&  eye,
                          const Point<Float, 3>&  center,
                          const Vector<Float, 3>& up)
  {
    return em::lookAt<Matrix>(eye, center, up);
  }

  template <size_t S = Size>
    requires(S == 4)
  static Transform rotate(const Vector<Float, Size - 1>& axis,
                          const Float&                   angle)
  {
    return em::rotate<Matrix>(axis, angle);
  }
};
} // namespace eldr::em
