#pragma once
#include "arrayrouter.hpp"

#include "arrayiface.hpp"

NAMESPACE_BEGIN(eldr::embr)

template <typename Value_, size_t _Sz>
struct Matrix
  : StaticArray<Array<Value_, _Sz>, _Sz, false, Matrix<Value_, _Sz>> {
  using Row = Array<Value_, _Sz>;
  using Base = StaticArray<Row, _Sz, false, Matrix<Value_, _Sz>>;
  using Entry = Value_;

  template <typename T> using ReplaceValue = Matrix<T, _Sz>;

  using Base::entry;
  static constexpr size_t kSize = _Sz;

  static constexpr bool kIsMatrix = true;
  static constexpr bool kIsSpecial = true;

  EL_ARRAY_DEFAULTS(Matrix)
  Matrix() = default;

  /// Construct from another matrix (possibly different size).
  /// If the argument is larger, retains the top-left block.
  /// If smaller, copies the top-left block and pads with identity.
  template <typename T>
    requires(is_matrix_v<std::decay_t<T>>)
  Matrix(T&& m)
  {
    constexpr size_t ArgSize = size_v<std::decay_t<T>>;
    if constexpr (ArgSize >= kSize) {
      for (size_t i = 0; i < kSize; ++i)
        entry(i) = head<kSize>(m.entry(i));
    }
    else {
      using Remainder = Array<Entry, kSize - ArgSize>;
      for (size_t i = 0; i < ArgSize; ++i)
        entry(i) = Row(concat(m.entry(i), zeros<Remainder>()));
      for (size_t i = ArgSize; i < kSize; ++i) {
        entry(i) = zeros<Row>();
        entry(i, i) = Entry(1);
      }
    }
  }

  /// Diagonal/identity constructor: Matrix(v) puts v on the diagonal
  template <typename T>
    requires(!is_array_v<T> && std::is_convertible_v<T, Entry>)
  explicit Matrix(T v) : Base(zeros<Entry>())
  {
    for (size_t i = 0; i < kSize; ++i)
      entry(i, i) = Entry(v);
  }

  /// Construct from rows
  template <typename... Args>
    requires(sizeof...(Args) == _Sz &&
             (std::is_constructible_v<Row, Args> && ...))
  Matrix(const Args&... args) : Base(args...)
  {
  }

  /// Construct from flat entries in row-major order
  template <typename... Args>
    requires(sizeof...(Args) == _Sz * _Sz &&
             (std::is_convertible_v<Args, Entry> && ...))
  Matrix(const Args&... args)
  {
    Entry values[sizeof...(Args)] = { Entry(args)... };
    for (size_t i = 0; i < kSize; ++i)
      for (size_t j = 0; j < kSize; ++j)
        entry(i, j) = values[i * kSize + j];
  }

  /// 2D element access
  EL_INLINE Entry&       entry(size_t i, size_t j) { return entry(i).entry(j); }
  EL_INLINE const Entry& entry(size_t i, size_t j) const
  {
    return entry(i).entry(j);
  }

  /// Operator() for 2D access
  EL_INLINE Entry&       operator()(size_t i, size_t j) { return entry(i, j); }
  EL_INLINE const Entry& operator()(size_t i, size_t j) const
  {
    return entry(i, j);
  }

  /// Row access
  EL_INLINE Row&       operator[](size_t i) { return entry(i); }
  EL_INLINE const Row& operator[](size_t i) const { return entry(i); }

  /// Static identity matrix
  static Matrix identity() { return Matrix(Entry(1)); }
};

// -----------------------------------------------------------------------------
// Matrix-Matrix multiplication
// -----------------------------------------------------------------------------
template <typename T0, typename T1, size_t Size>
EL_INLINE Matrix<T0, Size> operator*(const Matrix<T0, Size>& m0,
                                     const Matrix<T1, Size>& m1)
{
  using Result = Matrix<T0, Size>;
  using Row = typename Result::Row;

  Result result;
  for (size_t i = 0; i < Size; ++i) {
    Row row = m0(i, 0) * m1.entry(0);
    for (size_t j = 1; j < Size; ++j)
      row = fmadd(Row(m0(i, j)), m1.entry(j), row);
    result.entry(i) = row;
  }
  return result;
}

// -----------------------------------------------------------------------------
// Matrix-Vector multiplication
// -----------------------------------------------------------------------------
template <typename T0, typename T1, size_t Size>
  requires(is_array_v<T1> && !is_matrix_v<T1> && size_v<T1> == Size)
EL_INLINE auto operator*(const Matrix<T0, Size>& m, const T1& v)
{
  using Result = Array<expr_t<T0, value_t<T1>>, Size>;

  // Use transpose + fma pattern
  Matrix<T0, Size> t = transpose(m);
  Result result = t.entry(0) * Result(v.entry(0)); // Broadcast scalar to array
  for (size_t i = 1; i < Size; ++i)
    result = fmadd(t.entry(i), Result(v.entry(i)), result);
  return result;
}

// -----------------------------------------------------------------------------
// Scalar-Matrix and Matrix-Scalar operations
// -----------------------------------------------------------------------------
template <typename T0, typename T1, size_t Size>
  requires(!is_array_v<T0>)
EL_INLINE Matrix<T1, Size> operator*(const T0&               scalar,
                                     const Matrix<T1, Size>& m)
{
  Matrix<T1, Size> result;
  for (size_t i = 0; i < Size; ++i)
    result.entry(i) = scalar * m.entry(i);
  return result;
}

template <typename T0, typename T1, size_t Size>
  requires(!is_array_v<T1>)
EL_INLINE Matrix<T0, Size> operator*(const Matrix<T0, Size>& m,
                                     const T1&               scalar)
{
  return scalar * m;
}

// -----------------------------------------------------------------------------
// Transpose
// -----------------------------------------------------------------------------
template <typename Value, size_t Size>
EL_INLINE Matrix<Value, Size> transpose(const Matrix<Value, Size>& m)
{
  Matrix<Value, Size> result;
  for (size_t i = 0; i < Size; ++i)
    for (size_t j = 0; j < Size; ++j)
      result(i, j) = m(j, i);
  return result;
}

// -----------------------------------------------------------------------------
// Trace (sum of diagonal elements)
// -----------------------------------------------------------------------------
template <typename Value, size_t Size>
EL_INLINE Value trace(const Matrix<Value, Size>& m)
{
  Value result = m(0, 0);
  for (size_t i = 1; i < Size; ++i)
    result += m(i, i);
  return result;
}

// -----------------------------------------------------------------------------
// Diagonal extraction
// -----------------------------------------------------------------------------
template <typename Value, size_t Size>
EL_INLINE Array<Value, Size> diag(const Matrix<Value, Size>& m)
{
  Array<Value, Size> result;
  for (size_t i = 0; i < Size; ++i)
    result.entry(i) = m(i, i);
  return result;
}

// -----------------------------------------------------------------------------
// Create diagonal matrix from vector
// -----------------------------------------------------------------------------
template <typename T>
  requires(is_array_v<T> && !is_matrix_v<T>)
EL_INLINE Matrix<value_t<T>, size_v<T>> diag(const T& v)
{
  using Result = Matrix<value_t<T>, size_v<T>>;
  Result result(value_t<T>(0));
  for (size_t i = 0; i < size_v<T>; ++i)
    result(i, i) = v.entry(i);
  return result;
}

// -----------------------------------------------------------------------------
// Determinant
// -----------------------------------------------------------------------------

template <typename T> EL_INLINE T det(const Matrix<T, 1>& m) { return m(0, 0); }

template <typename T> EL_INLINE T det(const Matrix<T, 2>& m)
{
  return fmsub(m(0, 0), m(1, 1), m(0, 1) * m(1, 0));
}

template <typename T> EL_INLINE T det(const Matrix<T, 3>& m)
{
  return dot(m.entry(0), cross(m.entry(1), m.entry(2)));
}

template <typename T> EL_INLINE T det(const Matrix<T, 4>& m)
{
  using Vector = Array<T, 4>;

  Vector row0 = m.entry(0), row1 = m.entry(1), row2 = m.entry(2),
         row3 = m.entry(3);

  row1 = shuffle<2, 3, 0, 1>(row1);
  row3 = shuffle<2, 3, 0, 1>(row3);

  Vector temp, col0;

  temp = shuffle<1, 0, 3, 2>(row2 * row3);
  col0 = row1 * temp;
  temp = shuffle<2, 3, 0, 1>(temp);
  col0 = fmsub(row1, temp, col0);

  temp = shuffle<1, 0, 3, 2>(row1 * row2);
  col0 = fmadd(row3, temp, col0);
  temp = shuffle<2, 3, 0, 1>(temp);
  col0 = fnmadd(row3, temp, col0);

  row1 = shuffle<2, 3, 0, 1>(row1);
  row2 = shuffle<2, 3, 0, 1>(row2);
  temp = shuffle<1, 0, 3, 2>(row1 * row3);
  col0 = fmadd(row2, temp, col0);
  temp = shuffle<2, 3, 0, 1>(temp);
  col0 = fnmadd(row2, temp, col0);

  return dot(row0, col0);
}

// -----------------------------------------------------------------------------
// Inverse transpose
// -----------------------------------------------------------------------------

template <typename T>
EL_INLINE Matrix<T, 1> inverse_transpose(const Matrix<T, 1>& m)
{
  return Matrix<T, 1>(rcp(m(0, 0)));
}

template <typename T>
EL_INLINE Matrix<T, 2> inverse_transpose(const Matrix<T, 2>& m)
{
  T inv_det = rcp(det(m));
  return Matrix<T, 2>(m(1, 1) * inv_det,
                      -m(1, 0) * inv_det,
                      -m(0, 1) * inv_det,
                      m(0, 0) * inv_det);
}

template <typename T>
EL_INLINE Matrix<T, 3> inverse_transpose(const Matrix<T, 3>& m)
{
  using Vector = Array<T, 3>;

  Vector row0 = m.entry(0), row1 = m.entry(1), row2 = m.entry(2);

  Vector col0 = cross(row1, row2);
  Vector col1 = cross(row2, row0);
  Vector col2 = cross(row0, row1);

  T inv_det = rcp(dot(row0, col0));
  return Matrix<T, 3>(col0 * inv_det, col1 * inv_det, col2 * inv_det);
}

template <typename T>
EL_INLINE Matrix<T, 4> inverse_transpose(const Matrix<T, 4>& m)
{
  using Vector = Array<T, 4>;

  Vector row0 = m.entry(0), row1 = m.entry(1), row2 = m.entry(2),
         row3 = m.entry(3);

  row1 = shuffle<2, 3, 0, 1>(row1);
  row3 = shuffle<2, 3, 0, 1>(row3);

  Vector temp, col0, col1, col2, col3;

  temp = shuffle<1, 0, 3, 2>(row2 * row3);
  col0 = row1 * temp;
  col1 = row0 * temp;
  temp = shuffle<2, 3, 0, 1>(temp);
  col0 = fmsub(row1, temp, col0);
  col1 = shuffle<2, 3, 0, 1>(fmsub(row0, temp, col1));

  temp = shuffle<1, 0, 3, 2>(row1 * row2);
  col0 = fmadd(row3, temp, col0);
  col3 = row0 * temp;
  temp = shuffle<2, 3, 0, 1>(temp);
  col0 = fnmadd(row3, temp, col0);
  col3 = shuffle<2, 3, 0, 1>(fmsub(row0, temp, col3));

  temp = shuffle<1, 0, 3, 2>(shuffle<2, 3, 0, 1>(row1) * row3);
  row2 = shuffle<2, 3, 0, 1>(row2);
  col0 = fmadd(row2, temp, col0);
  col2 = row0 * temp;
  temp = shuffle<2, 3, 0, 1>(temp);
  col0 = fnmadd(row2, temp, col0);
  col2 = shuffle<2, 3, 0, 1>(fmsub(row0, temp, col2));

  temp = shuffle<1, 0, 3, 2>(row0 * row1);
  col2 = fmadd(row3, temp, col2);
  col3 = fmsub(row2, temp, col3);
  temp = shuffle<2, 3, 0, 1>(temp);
  col2 = fmsub(row3, temp, col2);
  col3 = fnmadd(row2, temp, col3);

  temp = shuffle<1, 0, 3, 2>(row0 * row3);
  col1 = fnmadd(row2, temp, col1);
  col2 = fmadd(row1, temp, col2);
  temp = shuffle<2, 3, 0, 1>(temp);
  col1 = fmadd(row2, temp, col1);
  col2 = fnmadd(row1, temp, col2);

  temp = shuffle<1, 0, 3, 2>(row0 * row2);
  col1 = fmadd(row3, temp, col1);
  col3 = fnmadd(row1, temp, col3);
  temp = shuffle<2, 3, 0, 1>(temp);
  col1 = fnmadd(row3, temp, col1);
  col3 = fmadd(row1, temp, col3);

  T inv_det = rcp(dot(row0, col0));

  return Matrix<T, 4>(
    col0 * inv_det, col1 * inv_det, col2 * inv_det, col3 * inv_det);
}

// -----------------------------------------------------------------------------
// Inverse
// -----------------------------------------------------------------------------

template <typename T> EL_INLINE Matrix<T, 1> inverse(const Matrix<T, 1>& m)
{
  return Matrix<T, 1>(rcp(m(0, 0)));
}

template <typename T> EL_INLINE Matrix<T, 2> inverse(const Matrix<T, 2>& m)
{
  T inv_det = rcp(det(m));
  return Matrix<T, 2>(m(1, 1) * inv_det,
                      -m(0, 1) * inv_det,
                      -m(1, 0) * inv_det,
                      m(0, 0) * inv_det);
}

template <typename T, size_t Size>
  requires(Size >= 3)
EL_INLINE Matrix<T, Size> inverse(const Matrix<T, Size>& m)
{
  return transpose(inverse_transpose(m));
}

template <typename T> using entry_t = typename T::Entry;

NAMESPACE_END(eldr::embr)
