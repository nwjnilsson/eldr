#pragma once
#include <eldr/arrays/arraybase.hpp>
#include <utility>

NAMESPACE_BEGIN(eldr::arr)

template <typename _Val, size_t _Size, typename _Derived>
struct StaticArrayBase
  : ArrayBase<_Val, StaticArrayBase<_Val, _Size, _Derived>> {
  using Base = ArrayBase<_Val, StaticArrayBase<_Val, _Size, _Derived>>;
  EL_ARRAY_IMPORT(StaticArrayBase, Base)

  using typename Base::Derived;
  using typename Base::Scalar;
  using typename Base::Value;

  using Base::derived;

  static constexpr size_t Size{ _Size };

  static constexpr size_t ActualSize{ Size };

  /// Recursive array indexing operator
  template <typename... Indices>
    requires(sizeof...(Indices) >= 1)
  EL_INLINE decltype(auto) entry(size_t i0, Indices... indices)
  {
    return derived().entry(i0).entry(indices...);
  }

  /// Recursive array indexing operator (const)
  template <typename... Indices>
    requires(sizeof...(Indices) >= 1)
  EL_INLINE decltype(auto) entry(size_t i0, Indices... indices) const
  {
    return derived().entry(i0).entry(indices...);
  }

  /// Array indexing operator with bounds checks in debug mode
  EL_INLINE decltype(auto) operator[](size_t i)
  {
#if !defined(NDEBUG) && !defined(EL_DISABLE_RANGE_CHECK)
    if (i >= derived().size())
      arr_fail("ArrayBase: out of range access (tried to "
               "access index %zu in an array of size %zu)",
               i,
               derived().size());
#endif
    return derived().entry(i);
  }

  /// Array indexing operator with bounds checks in debug mode, const version
  EL_INLINE decltype(auto) operator[](size_t i) const
  {
#if !defined(NDEBUG) && !defined(EL_DISABLE_RANGE_CHECK)
    if (i >= derived().size())
      arr_fail("ArrayBase: out of range access (tried to "
               "access index %zu in an array of size %zu)",
               i,
               derived().size());
#endif
    return derived().entry(i);
  }

  template <typename T> EL_INLINE void setEntry(size_t i, T&& value)
  {
    derived().entry(i) = std::forward<T>(value);
  }

  EL_INLINE bool empty() const { return derived().size() == 0; }

  EL_INLINE decltype(auto) x() const
  {
    static_assert(Derived::ActualSize >= 1,
                  "StaticArrayBase::x(): requires Size >= 1");
    return derived().entry(0);
  }

  EL_INLINE decltype(auto) x()
  {
    static_assert(Derived::ActualSize >= 1,
                  "StaticArrayBase::x(): requires Size >= 1");
    return derived().entry(0);
  }

  EL_INLINE decltype(auto) y() const
  {
    static_assert(Derived::ActualSize >= 2,
                  "StaticArrayBase::y(): requires Size >= 2");
    return derived().entry(1);
  }

  EL_INLINE decltype(auto) y()
  {
    static_assert(Derived::ActualSize >= 2,
                  "StaticArrayBase::y(): requires Size >= 2");
    return derived().entry(1);
  }

  EL_INLINE decltype(auto) z() const
  {
    static_assert(Derived::ActualSize >= 3,
                  "StaticArrayBase::z(): requires Size >= 3");
    return derived().entry(2);
  }

  EL_INLINE decltype(auto) z()
  {
    static_assert(Derived::ActualSize >= 3,
                  "StaticArrayBase::z(): requires Size >= 3");
    return derived().entry(2);
  }

  EL_INLINE decltype(auto) w() const
  {
    static_assert(Derived::ActualSize >= 4,
                  "StaticArrayBase::w(): requires Size >= 4");
    return derived().entry(3);
  }
};
NAMESPACE_END(eldr::arr)
