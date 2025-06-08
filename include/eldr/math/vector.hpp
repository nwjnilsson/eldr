#pragma once
#include <eldr/math/arraygeneric.hpp>

namespace eldr::em {
template <typename Value_, size_t Size_>
struct Vector : StaticArray<Value_, Size_, Vector<Value_, Size_>> {
  using Base = StaticArray<Value_, Size_, Vector<Value_, Size_>>;
  EL_ARRAY_IMPORT(Vector, Base)

  using ArrayType = Vector;
};

template <typename Value_, size_t Size_>
struct Point : StaticArray<Value_, Size_, Point<Value_, Size_>> {
  using Base = StaticArray<Value_, Size_, Point<Value_, Size_>>;
  EL_ARRAY_IMPORT(Point, Base)

  using ArrayType = Point;

  // template <typename... Args>
  // Point(Args&&... args) : Base(std::forward(args...))
  // {
  // }
};

template <typename Value_, size_t Size_>
struct Normal : StaticArray<Value_, Size_, Normal<Value_, Size_>> {
  using Base = StaticArray<Value_, Size_, Normal<Value_, Size_>>;
  EL_ARRAY_IMPORT(Normal, Base)

  using ArrayType = Normal;
};

} // namespace eldr::em
