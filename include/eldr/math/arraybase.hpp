/// The code in the comment below is a watered down version of the array
/// implementation in Dr.Jit. I decided to keep it here as a comment in case I
/// want to try to roll my own array implementation with JIT compilation in the
/// future, but for now I'll just use glm arrays from arraygeneric.hpp

#include <eldr/core/platform.hpp>
#include <eldr/math/arraytraits.hpp>
// #include <utility>

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
//
// template <typename Value_, typename Derived_> struct ArrayBaseT {
//   using Value   = Value_;
//   using Derived = Derived_;
//   using Scalar  = scalar_t<Value_>;
//
//   static constexpr bool IsEldr{ true };
//   static constexpr bool IsVector{ false };
//   static constexpr bool IsMatrix{ false };
//   static constexpr bool IsQuaternion{ false };
//
//   /// Cast to derived type
//   EL_INLINE Derived& derived() { return static_cast<Derived&>(*this); }
//
//   /// Cast to derived type (const version)
//   EL_INLINE const Derived& derived() const
//   {
//     return static_cast<Derived&>(*this);
//   }
//
//   // -----------------------------------------------------------------------
//   //! @{ \name Iterators
//   // -----------------------------------------------------------------------
//
//   EL_INLINE auto begin() const { return derived().data(); }
//   EL_INLINE auto begin() { return derived().data(); }
//   EL_INLINE auto end() const { return derived().data() + derived().size(); }
//   EL_INLINE auto end() { return derived().data() + derived().size(); }
//
//   //! @}
//   // -----------------------------------------------------------------------
//
//   // -----------------------------------------------------------------------
//   //! @{ \name Element access
//   // -----------------------------------------------------------------------
//
//   /// Recursive array indexing operator
//   template <typename... Indices>
//     requires(sizeof...(Indices) >= 1)
//   EL_INLINE decltype(auto) entry(size_t i0, Indices... indices)
//   {
//     return derived().entry(i0).entry(indices...);
//   }
//
//   /// Recursive array indexing operator (const)
//   template <typename... Indices>
//     requires(sizeof...(Indices) >= 1)
//   EL_INLINE decltype(auto) entry(size_t i0, Indices... indices) const
//   {
//     return derived().entry(i0).entry(indices...);
//   }
//
//   /// Array indexing operator with bounds checks in debug mode
//   EL_INLINE decltype(auto) operator[](size_t i)
//   {
// #if !defined(NDEBUG)
//     if (i >= derived().size())
//       Throw("ArrayBaseT: out of range access (tried to "
//             "access index %zu in an array of size %zu)",
//             i,
//             derived().size());
// #endif
//     return derived().entry(i);
//   }
//
//   /// Array indexing operator with bounds checks in debug mode, const version
//   EL_INLINE decltype(auto) operator[](size_t i) const
//   {
// #ifdef DEBUG
//     if (i >= derived().size())
//       Throw("ArrayBaseT: out of range access (tried to "
//             "access index %zu in an array of size %zu)",
//             i,
//             derived().size());
// #endif
//     return derived().entry(i);
//   }
//
//   template <typename T> EL_INLINE void set_entry(size_t i, T&& value)
//   {
//     derived().entry(i) = std::forward<T>(value);
//   }
// };
//
} // namespace eldr::em
// template <typename T, size_t Size>
// struct std::hash<eldr::em::ArrayBaseT<T, Size>> {
//   size_t operator()(const eldr::em::ArrayBaseT<T, Size>& arr)
//   {
//     return std::hash<typename eldr::em::ArrayBaseT<T, Size>::Base>(arr)();
//   }
// };
