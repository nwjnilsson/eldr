#pragma once
#include <eldr/math/arraygeneric.hpp>

namespace eldr::core {
template <typename Value_, size_t Size_>
struct Color : em::StaticArray<Value_, Size_, Color<Value_, Size_>> {
  using Base = em::StaticArray<Value_, Size_, Color<Value_, Size_>>;
  EL_ARRAY_IMPORT(Color, Base)

  using typename Base::Scalar;
  using ArrayType = Color;

  decltype(auto) r() const { return Base::x(); }
  decltype(auto) r() { return Base::x(); }

  decltype(auto) g() const { return Base::y(); }
  decltype(auto) g() { return Base::y(); }

  decltype(auto) b() const { return Base::z(); }
  decltype(auto) b() { return Base::z(); }

  decltype(auto) a() const { return Base::w(); }
  decltype(auto) a() { return Base::w(); }
};

template <typename Value_, size_t Size_>
struct Spectrum : em::StaticArray<Value_, Size_, Spectrum<Value_, Size_>> {
  using Base = em::StaticArray<Value_, Size_, Spectrum<Value_, Size_>>;
  EL_ARRAY_IMPORT(Spectrum, Base)

  using ArrayType = Spectrum;
  using typename Base::Scalar;
};
} // namespace eldr::core
