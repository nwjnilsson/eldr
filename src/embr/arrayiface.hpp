#pragma once
#include "arraybase.hpp"

NAMESPACE_BEGIN(eldr::embr)
template <typename _Val, size_t _Size>
struct Array : ArrayBase<_Val, Array<_Val, _Size>> {
  using Base = ArrayBase<_Val, Array<_Val, _Size>>;
  EL_ARRAY_IMPORT(Array, Base)
};
NAMESPACE_END(eldr::embr)
