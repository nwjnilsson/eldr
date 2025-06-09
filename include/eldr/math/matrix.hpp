#pragma once
#include <eldr/math/arrayinterface.hpp>
#include <eldr/math/arraystatic.hpp>
#include <eldr/math/glm.hpp>

namespace eldr::em {

template <typename _Val, size_t _Size>
struct Matrix
  : StaticArray<Array<_Val, _Size>, _Size, Matrix<Array<_Val, _Size>, _Size>> {
  using Base =
    StaticArray<Array<_Val, _Size>, _Size, Matrix<Array<_Val, _Size>, _Size>>;

  using Row = Array<_Val, _Size>;

  using Base::Size;

  using Base::entry;
  // using Entry = _Val;
  //  EL_ARRAY_DEFAULTS(Matrix)
  //  EL_ARRAY_IMPORT(Matrix, Base)
  static constexpr bool IsMatrix{ true };
  // static constexpr bool IsSpecial{ true };
  static constexpr bool IsVector{ false };

  using ArrayType      = Matrix;
  using PlainArrayType = Array<Array<_Val, Size>, Size>;
  // using MaskType       = Mask<mask_t<Row>, Size_>;
  using Entry = _Val;

  constexpr Matrix() = default;

  template <typename T>
    requires(is_matrix_v<T> or depth_v<T> == Base::Depth)
  EL_INLINE Matrix(T&& m)
  {
    constexpr size_t ArgSize = size_v<T>;
    if constexpr (ArgSize >= Size) {
      /// Other matrix is equal or bigger -- retain the top left part
      for (size_t i = 0; i < Size; ++i)
        entry(i) = head<Size>(m.entry(i));
    }
    else {
      /// Other matrix is smaller -- copy the top left part and set remainder to
      /// identity
      using Remainder = Array<_Val, Size - ArgSize>;
      for (size_t i = 0; i < ArgSize; ++i)
        entry(i) = concat(m.entry(i), zeros<Remainder>());
      for (size_t i = ArgSize; i < Size; ++i) {
        Row col      = zeros<Row>();
        col.entry(i) = 1;
        entry(i)     = col;
      }
    }
  }

  template <typename T>
    requires(not is_matrix_v<T> and depth_v<T> != Base::Depth)
  EL_INLINE Matrix(T&& v) : Base(zeros<_Val>())
  {
    for (size_t i = 0; i < Size; ++i)
      entry(i, i) = v;
  }

  /// Initialize the matrix from a list of rows
  template <typename... Args>
    requires(sizeof...(Args) == Size and
             std::conjunction_v<std::is_constructible<Row, Args>...>)
  EL_INLINE Matrix(const Args&... args) : Base(args...)
  {
  }

  /// Initialize the matrix from a list of entries in row-major order
  template <typename... Args>
    requires(sizeof...(Args) == Size * Size and
             std::conjunction_v<std::is_constructible<_Val, Args>...>)
  EL_INLINE Matrix(const Args&... args)
  {
    _Val values[sizeof...(Args)]{ _Val(args)... };
    for (size_t i = 0; i < Size; ++i)
      for (size_t j = 0; j < Size; ++j)
        entry(i, j) = values[i * Size + j];
  }

  /// Return a reference to the (i, j) element
  EL_INLINE _Val& operator()(size_t i, size_t j) { return entry(i, j); }

  /// Return a reference to the (i, j) element (const)
  EL_INLINE const _Val& operator()(size_t i, size_t j) const
  {
    return entry(i, j);
  }
};

/// Type trait to access the entry type of a matrix
template <typename T> using entry_t = typename T::Entry;

template <typename T>
  requires is_matrix_v<T>
T identity(size_t size = 1)
{
  using Entry = value_t<value_t<T>>;
  Entry e{ identity<Entry>(size) };
  T     result{ zeros<T>(size) };
  for (size_t i{ 0 }; i < size_v<T>; ++i) {
    result.entry(i, i) = e;
  }
  return result;
}

} // namespace eldr::em
