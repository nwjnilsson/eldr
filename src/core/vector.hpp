#pragma once
#include <embr/array.hpp>

NAMESPACE_BEGIN(eldr)
template <typename _Val, size_t _Sz>
struct Vector : embr::StaticArray<_Val, _Sz, false, Vector<_Val, _Sz>> {
  using Base = embr::StaticArray<_Val, _Sz, false, Vector<_Val, _Sz>>;
  template <typename T> using ReplaceValue = Vector<T, _Sz>;
  EL_ARRAY_IMPORT(Vector, Base);
};

template <typename _Val, size_t _Sz>
struct Point : embr::StaticArray<_Val, _Sz, false, Point<_Val, _Sz>> {
  using Base = embr::StaticArray<_Val, _Sz, false, Point<_Val, _Sz>>;
  template <typename T> using ReplaceValue = Point<T, _Sz>;
  EL_ARRAY_IMPORT(Point, Base);
};

template <typename _Val, size_t _Sz>
struct Normal : embr::StaticArray<_Val, _Sz, false, Normal<_Val, _Sz>> {
  using Base = embr::StaticArray<_Val, _Sz, false, Normal<_Val, _Sz>>;
  template <typename T> using ReplaceValue = Normal<T, _Sz>;
  EL_ARRAY_IMPORT(Normal, Base);
};
NAMESPACE_END(eldr)

EL_ARRAY_HASH(eldr::Vector)
EL_ARRAY_HASH(eldr::Point)
EL_ARRAY_HASH(eldr::Normal)
