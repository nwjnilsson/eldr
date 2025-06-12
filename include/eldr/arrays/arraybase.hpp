#pragma once
#include <eldr/arrays/arrayutils.hpp>
#include <eldr/arrays/traits.hpp>

NAMESPACE_BEGIN(eldr::arr)

#define EL_ARRAY_DEFAULTS(Name)                                                \
  Name(const Name&)            = default;                                      \
  Name(Name&&)                 = default;                                      \
  Name& operator=(const Name&) = default;                                      \
  Name& operator=(Name&&)      = default;

#define EL_ARRAY_IMPORT(Name, Base)                                            \
  Name() = default;                                                            \
  EL_ARRAY_DEFAULTS(Name)                                                      \
  using Base::Base;

template <typename _Val, typename _Derived> struct ArrayBase {
  using Derived = _Derived;
  using Value   = _Val;
  using Scalar  = scalar_t<Value>;

  static constexpr bool IsArray{ true };

  Derived&       derived() { return static_cast<Derived>(*this); }
  Derived const& derived() const { return static_cast<Derived>(*this); }
};
NAMESPACE_END(eldr::arr)
