#pragma once
#include <eldr/arrays/arraystatic.hpp>
#include <eldr/math/glm.hpp>

NAMESPACE_BEGIN(eldr::arr)
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

  /// Generic constructor forwarding arguments to glm vector
  template <typename... Ts>
  StaticArray(Ts&&... ts) : array(std::forward<Ts>(ts)...){};

  /// Access elements by reference, and without error-checking
  EL_INLINE Value& entry(size_t i) { return array[i]; }

  /// Access elements by reference, and without error-checking (const)
  EL_INLINE const Value& entry(size_t i) const { return array[i]; }

  /// Pointer to the underlying storage
  Value* data() { return array; }

  /// Pointer to the underlying storage (const)
  const Value* data() const { return array; }

  using glm_t = glm::vec<Size, Value>;
  operator glm_t*() const { return array; }
  operator glm_t*() { return array; }
  operator glm_t() const { return array; }
  operator glm_t() { return array; }

  glm_t array;
};
NAMESPACE_END(eldr::arr)
