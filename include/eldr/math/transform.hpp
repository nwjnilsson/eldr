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

  Float&       operator()(size_t i, size_t j) { return matrix[i][j]; }
  const Float& operator()(size_t i, size_t j) const { return matrix[i][j]; }

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

// glm implementation of lookAt
template <typename Matrix>
  requires is_matrix_v<Matrix>
Matrix lookAt(const Array<entry_t<Matrix>, size_v<Matrix> - 1>& eye,
              const Array<entry_t<Matrix>, size_v<Matrix> - 1>& center,
              const Array<entry_t<Matrix>, size_v<Matrix> - 1>& up)
{
  using T               = entry_t<Matrix>;
  constexpr size_t Size = size_v<Matrix>;
  static_assert(Size == 4, "Template argument must be a 4x4 matrix");

  Array<T, Size> const f(normalize(center - eye));
  Array<T, Size> const s(normalize(cross(up, f)));
  Array<T, Size> const u(cross(f, s));

  Matrix result{ 1 };
  result(0, 0) = s.x;
  result(1, 0) = s.y;
  result(2, 0) = s.z;
  result(0, 1) = u.x;
  result(1, 1) = u.y;
  result(2, 1) = u.z;
  result(0, 2) = f.x;
  result(1, 2) = f.y;
  result(2, 2) = f.z;
  result(3, 0) = -dot(s, eye);
  result(3, 1) = -dot(u, eye);
  result(3, 2) = -dot(f, eye);
  return result;
}

template <typename Matrix>
  requires(is_matrix_v<Matrix> and size_v<Matrix> == 4)
Matrix rotate(const Array<entry_t<Matrix>, 3>& axis,
              const entry_t<Matrix>&           angle)
{
  using T = entry_t<Matrix>;

  T const a = angle;
  T const c = cos(a);
  T const s = sin(a);

  using Vector3 = Array<T, 3>;
  Vector3 norm_axis{ normalize(axis) };
  Vector3 temp{ (T{ 1 } - c) * norm_axis };

  Matrix rotation;
  rotation[0][0] = c + temp[0] * norm_axis[0];
  rotation[0][1] = temp[0] * norm_axis[1] + s * axis[2];
  rotation[0][2] = temp[0] * norm_axis[2] - s * axis[1];

  rotation[1][0] = temp[1] * norm_axis[0] - s * axis[2];
  rotation[1][1] = c + temp[1] * norm_axis[1];
  rotation[1][2] = temp[1] * norm_axis[2] + s * axis[0];

  rotation[2][0] = temp[2] * norm_axis[0] + s * axis[1];
  rotation[2][1] = temp[2] * norm_axis[1] - s * axis[0];
  rotation[2][2] = c + temp[2] * norm_axis[2];

  Matrix result;
  Matrix identity{ identity() };
  result[0] = identity[0] * rotation[0][0] + identity[1] * rotation[0][1] +
              identity[2] * rotation[0][2];
  result[1] = identity[0] * rotation[1][0] + identity[1] * rotation[1][1] +
              identity[2] * rotation[1][2];
  result[2] = identity[0] * rotation[2][0] + identity[1] * rotation[2][1] +
              identity[2] * rotation[2][2];
  result[3] = identity[3];
  return result;
}
} // namespace eldr::em
