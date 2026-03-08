#pragma once
#include "arraybase.hpp"

NAMESPACE_BEGIN(eldr::embr)

template <typename _Val, size_t _Sz, bool _IsMask, typename _Derived>
struct StaticArrayBase : ArrayBase<_Val, _IsMask, _Derived> {
  using Base = ArrayBase<_Val, _IsMask, _Derived>;
  EL_ARRAY_IMPORT(StaticArrayBase, Base)

  using typename Base::Derived;
  using typename Base::Scalar;
  using typename Base::Value;

  using Base::derived;

  static constexpr size_t kSize{ _Sz };

  static constexpr size_t kActualSize{ kSize };

  static constexpr size_t size() { return Derived::kSize; }

  EL_INLINE decltype(auto) x() const
  {
    static_assert(Derived::kActualSize >= 1,
                  "StaticArrayBase::x(): requires Size >= 1");
    return derived().entry(0);
  }

  EL_INLINE decltype(auto) x()
  {
    static_assert(Derived::kActualSize >= 1,
                  "StaticArrayBase::x(): requires Size >= 1");
    return derived().entry(0);
  }

  EL_INLINE decltype(auto) y() const
  {
    static_assert(Derived::kActualSize >= 2,
                  "StaticArrayBase::y(): requires Size >= 2");
    return derived().entry(1);
  }

  EL_INLINE decltype(auto) y()
  {
    static_assert(Derived::kActualSize >= 2,
                  "StaticArrayBase::y(): requires Size >= 2");
    return derived().entry(1);
  }

  EL_INLINE decltype(auto) z() const
  {
    static_assert(Derived::kActualSize >= 3,
                  "StaticArrayBase::z(): requires Size >= 3");
    return derived().entry(2);
  }

  EL_INLINE decltype(auto) z()
  {
    static_assert(Derived::kActualSize >= 3,
                  "StaticArrayBase::z(): requires Size >= 3");
    return derived().entry(2);
  }

  EL_INLINE decltype(auto) w() const
  {
    static_assert(Derived::kActualSize >= 4,
                  "StaticArrayBase::w(): requires Size >= 4");
    return derived().entry(3);
  }

  EL_INLINE decltype(auto) w()
  {
    static_assert(Derived::kActualSize >= 4,
                  "StaticArrayBase::w(): requires Size >= 4");
    return derived().entry(3);
  }

};

NAMESPACE_END(eldr::embr)
