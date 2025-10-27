#pragma once
#include "math/arraygeneric.hpp"

NAMESPACE_BEGIN(eldr)
template <typename _Val, size_t _Size>
struct Vector : math::StaticArray<_Val, _Size, Vector<_Val, _Size>> {
  using Base = math::StaticArray<_Val, _Size, Vector<_Val, _Size>>;
  EL_ARRAY_IMPORT(Vector, Base);
};

template <typename _Val, size_t _Size>
struct Point : math::StaticArray<_Val, _Size, Vector<_Val, _Size>> {
  using Base = math::StaticArray<_Val, _Size, Vector<_Val, _Size>>;
  EL_ARRAY_IMPORT(Point, Base);
  using Base::derived;
  using Base::entry;
};
template <typename _Val, size_t _Size>
struct Normal : math::StaticArray<_Val, _Size, Vector<_Val, _Size>> {
  using Base = math::StaticArray<_Val, _Size, Vector<_Val, _Size>>;
  EL_ARRAY_IMPORT(Normal, Base);
};
NAMESPACE_END(eldr)
