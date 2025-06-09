#pragma once
#include <eldr/math/arraygeneric.hpp>

namespace eldr::em {
template <typename _Val, size_t _Size>
struct Array : StaticArray<_Val, _Size, Array<_Val, _Size>> {
  using Base = StaticArray<_Val, _Size, Array<_Val, _Size>>;
  EL_ARRAY_IMPORT(Array, Base)

  using ArrayType = Array<_Val, _Size>;
  // Type alias for creating a similar-shaped array over a different type
  template <typename T> using ReplaceValue = Array<T, _Size>;
};
} // namespace eldr::em
