#pragma once
#include "arraystatic.hpp"

NAMESPACE_BEGIN(eldr::math)
template <typename _Val, size_t _Size, typename _Derived>
struct StaticArray
  : StaticArrayBase<_Val, _Size, StaticArray<_Val, _Size, _Derived>> {
  using Base = StaticArrayBase<_Val, _Size, StaticArray<_Val, _Size, _Derived>>;
  EL_ARRAY_IMPORT(StaticArray, Base)

  using typename Base::Derived;
  using typename Base::Scalar;
  using typename Base::Value;

  using Base::derived;
  using Base::entry;
  using Base::Size;

  template <typename T>
    requires std::is_scalar_v<T>
  StaticArray(T v)
  {
    for (size_t i{ 0 }; i < Size; ++i) {
      array[i] = v;
    }
  }
  template <typename T = Value>
    requires(!std::is_same_v<T, Scalar>)
  StaticArray(const Value& v)
  {
    for (size_t i{ 0 }; i < Size; ++i) {
      array[i] = v;
    }
  }
  template <typename... Ts>
    requires(detail::is_components_v<Size, Ts...>)
  StaticArray(Ts&&... ts) : array{ move_cast_t<Ts, Value>(ts)... } {};

  /// Access elements by reference, and without error-checking
  EL_INLINE Value& entry(size_t i) { return array[i]; }

  /// Access elements by reference, and without error-checking (const)
  EL_INLINE const Value& entry(size_t i) const { return array[i]; }

  /// Pointer to the underlying storage
  Value* data() { return array; }

  /// Pointer to the underlying storage (const)
  const Value* data() const { return array; }

  // operator Value*() const { return array; }
  // operator Value*() { return array; }
  // operator Value() const { return array; }
  // operator Value() { return array; }

private:
  Value array[Size];
};
NAMESPACE_END(eldr::math)
