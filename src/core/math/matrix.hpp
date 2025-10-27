#pragma once
#include "arraygeneric.hpp"
#include "arrayiface.hpp"

NAMESPACE_BEGIN(eldr::math)
template <typename _Val, size_t _Size>
struct Matrix : StaticArray<Array<_Val, _Size>, _Size, Matrix<_Val, _Size>> {
  using Row  = Array<_Val, _Size>;
  using Base = StaticArrayBase<Row, _Size, Matrix<_Val, _Size>>;
  using Base::entry;
  using Base::Size;
  EL_ARRAY_DEFAULTS(Matrix)
};
NAMESPACE_END(eldr::math)
