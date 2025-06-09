#include <eldr/core/platform.hpp>
#include <eldr/math/arraytraits.hpp>
#include <eldr/math/arrayutils.hpp>
#include <utility>

#pragma once
namespace eldr::em {

#define EL_ARRAY_DEFAULTS(Name)                                                \
  constexpr Name(const Name& arg)     = default;                               \
  constexpr Name(Name&& arg) noexcept = default;                               \
  Name& operator=(const Name&)        = default;                               \
  Name& operator=(Name&&) noexcept    = default;

#define EL_ARRAY_IMPORT(Name, Base)                                            \
  constexpr Name() = default;                                                  \
  EL_ARRAY_DEFAULTS(Name)                                                      \
  using Base::Base;

template <typename _Val, typename _Derived> struct ArrayBase {
  using Value   = _Val;
  using Derived = _Derived;
  using Scalar  = scalar_t<_Val>;

  static constexpr bool IsArr{ true };
  static constexpr bool IsVector{ false };
  static constexpr bool IsMatrix{ false };
  static constexpr bool IsQuaternion{ false };

  /// Cast to derived type
  EL_INLINE Derived& derived() { return static_cast<Derived&>(*this); }

  /// Cast to derived type (const version)
  EL_INLINE const Derived& derived() const
  {
    return static_cast<Derived&>(*this);
  }

  // -----------------------------------------------------------------------
  //! @{ \name Iterators
  // -----------------------------------------------------------------------

  EL_INLINE auto begin() const { return derived().data(); }
  EL_INLINE auto begin() { return derived().data(); }
  EL_INLINE auto end() const { return derived().data() + derived().size(); }
  EL_INLINE auto end() { return derived().data() + derived().size(); }

  //! @}
  // -----------------------------------------------------------------------

  // -----------------------------------------------------------------------
  //! @{ \name Element access
  // -----------------------------------------------------------------------

  /// Recursive array indexing operator
  template <typename... _Indices>
    requires(sizeof...(_Indices) >= 1)
  EL_INLINE decltype(auto) entry(size_t i0, _Indices... indices)
  {
    return derived().entry(i0).entry(indices...);
  }

  /// Recursive array indexing operator (const)
  template <typename... _Indices>
    requires(sizeof...(_Indices) >= 1)
  EL_INLINE decltype(auto) entry(size_t i0, _Indices... indices) const
  {
    return derived().entry(i0).entry(indices...);
  }

  /// Array indexing operator with bounds checks in debug mode
  EL_INLINE decltype(auto) operator[](size_t i)
  {
#ifndef NDEBUG
    if constexpr (i >= derived().size())
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
#ifdef DEBUG
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
};

} // namespace eldr::em
// template <typename T, size_t size>
// struct std::hash<eldr::em::ArrayBase<T, size>> {
//   size_t operator()(const eldr::em::ArrayBase<T, size>& arr)
//   {
//     return std::hash<typename eldr::em::ArrayBase<T, size>::Base>(arr)();
//   }
// };
