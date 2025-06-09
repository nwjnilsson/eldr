#pragma once
#include <eldr/math/arraybase.hpp>

namespace eldr::em {

template <typename _Val, size_t _size, typename _Derived>
struct StaticArrayBase : ArrayBase<_Val, _Derived> {
  using Base = ArrayBase<_Val, _Derived>;
  EL_ARRAY_IMPORT(StaticArrayBase, Base);

  using typename Base::Derived;
  using typename Base::Scalar;
  using typename Base::Value;

  using Base::derived;

  static constexpr size_t Size{ _size };
  static constexpr bool   IsVector{ true };

  EL_INLINE constexpr size_t size() const { return Derived::size; }

  EL_INLINE decltype(auto) x() const
  {
    static_assert(Derived::ActualSize >= 1,
                  "StaticArrayBase::x(): requires size >= 1");
    return derived().entry(0);
  }

  EL_INLINE decltype(auto) x()
  {
    static_assert(Derived::ActualSize >= 1,
                  "StaticArrayBase::x(): requires size >= 1");
    return derived().entry(0);
  }

  EL_INLINE decltype(auto) y() const
  {
    static_assert(Derived::ActualSize >= 2,
                  "StaticArrayBase::y(): requires size >= 2");
    return derived().entry(1);
  }

  EL_INLINE decltype(auto) y()
  {
    static_assert(Derived::ActualSize >= 2,
                  "StaticArrayBase::y(): requires size >= 2");
    return derived().entry(1);
  }

  EL_INLINE decltype(auto) z() const
  {
    static_assert(Derived::ActualSize >= 3,
                  "StaticArrayBase::z(): requires size >= 3");
    return derived().entry(2);
  }

  EL_INLINE decltype(auto) z()
  {
    static_assert(Derived::ActualSize >= 3,
                  "StaticArrayBase::z(): requires size >= 3");
    return derived().entry(2);
  }

  EL_INLINE decltype(auto) w() const
  {
    static_assert(Derived::ActualSize >= 4,
                  "StaticArrayBase::w(): requires size >= 4");
    return derived().entry(3);
  }

  EL_INLINE decltype(auto) w()
  {
    static_assert(Derived::ActualSize >= 4,
                  "StaticArrayBase::w(): requires size >= 4");
    return derived().entry(3);
  }
};
} // namespace eldr::em
