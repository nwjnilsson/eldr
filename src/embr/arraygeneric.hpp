#pragma once
#include "arraystatic.hpp"

NAMESPACE_BEGIN(eldr::embr)
template <typename _Val, size_t _Sz, bool _IsMask, typename _Derived>
struct StaticArray : StaticArrayBase<_Val, _Sz, _IsMask, _Derived> {
  using Base = StaticArrayBase<_Val, _Sz, _IsMask, _Derived>;
  EL_ARRAY_IMPORT(StaticArray, Base)

  using typename Base::Derived;
  using typename Base::Scalar;
  using typename Base::Value;

  using Base::derived;
  using Base::entry;
  using Base::kSize;

  /// Construct from any other array type of matching size
  template <typename Value2, typename D2, typename D = _Derived>
    requires(D::kSize != D2::kSize || D::kDepth != D2::kDepth)
  StaticArray(const ArrayBase<Value2, _IsMask, D2>& v, detail::reinterpret_flag)
  {
    if constexpr (D::Size == D2::Size && D2::kBroadcastOuter) {
      static_assert(
        std::is_constructible_v<Value, value_t<D2>, detail::reinterpret_flag>);
      for (size_t i = 0; i < derived().size(); ++i)
        derived().entry(i) = reinterpret_array<Value>(v.derived().entry(i));
    }
    else {
      static_assert(
        std::is_constructible_v<Value, D2, detail::reinterpret_flag>);
      for (size_t i = 0; i < derived().size(); ++i)
        derived().entry(i) = reinterpret_array<Value>(v.derived());
    }
  }

  /// Scalar broadcast
  template <typename T>
    requires std::is_scalar_v<T>
  StaticArray(T v)
  {
    for (size_t i{ 0 }; i < kSize; ++i) {
      array[i] = v;
    }
  }

  template <typename T = Value>
    requires(!std::is_same_v<T, Scalar>)
  StaticArray(const Value& v)
  {
    for (size_t i{ 0 }; i < kSize; ++i) {
      array[i] = v;
    }
  }

  /// Construct from components
  template <typename... Ts>
    requires(detail::is_components_v<kSize, Ts...>)
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
  Value array[kSize];
};
NAMESPACE_END(eldr::embr)
