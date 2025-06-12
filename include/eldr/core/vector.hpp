#pragma once
#include <eldr/arrays/arraygeneric.hpp>

NAMESPACE_BEGIN(eldr::core)
template <typename _Val, size_t _Size>
struct Vector : arr::StaticArray<_Val, _Size, Vector<_Val, _Size>> {
  using Base = arr::StaticArray<_Val, _Size, Vector<_Val, _Size>>;
  EL_ARRAY_IMPORT(Vector, Base);
};

template <typename _Val, size_t _Size>
struct Point : arr::StaticArray<_Val, _Size, Vector<_Val, _Size>> {
  using Base = arr::StaticArray<_Val, _Size, Vector<_Val, _Size>>;
  EL_ARRAY_IMPORT(Point, Base);
  using Base::derived;
  using Base::entry;
};
template <typename _Val, size_t _Size>
struct Normal : arr::StaticArray<_Val, _Size, Vector<_Val, _Size>> {
  using Base = arr::StaticArray<_Val, _Size, Vector<_Val, _Size>>;
  EL_ARRAY_IMPORT(Normal, Base);
};
NAMESPACE_END(eldr::core)
