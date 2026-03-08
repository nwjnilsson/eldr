#pragma once
#include "arraygeneric.hpp"

NAMESPACE_BEGIN(eldr::embr)
template <typename _Val, size_t _Sz>
struct Array : StaticArray<_Val, _Sz, false, Array<_Val, _Sz>> {
  using Base = StaticArray<_Val, _Sz, false, Array<_Val, _Sz>>;

  template <typename T> using ReplaceValue = Array<T, _Sz>;

  EL_ARRAY_IMPORT(Array, Base)
};
NAMESPACE_END(eldr::embr)
