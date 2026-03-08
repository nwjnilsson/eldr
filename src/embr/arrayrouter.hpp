#pragma once
#include "arrayutils.hpp"
#include "traits.hpp"

#include <cmath>
#include <functional>
#include <utility>

NAMESPACE_BEGIN(eldr::embr)

// Forward declaration for concat() return type
template <typename _Val, size_t _Sz> struct Array;

// Router macros for dispatching operations to array members or scalar fallback

/// Unary operation router - dispatches to member function or scalar fallback
#define EL_ROUTE_UNARY(name, func)                                             \
  template <typename T>                                                        \
    requires is_array_v<T>                                                     \
  EL_INLINE auto name(const T& a)                                              \
  {                                                                            \
    return a.func##_();                                                        \
  }

/// Define an unary operation with a fallback expression for scalar arguments
#define EL_ROUTE_UNARY_FALLBACK(name, func, expr)                              \
  template <typename T> EL_INLINE auto name(const T& a)                        \
  {                                                                            \
    if constexpr (!is_array_v<T>)                                              \
      return expr; /* Scalar fallback implementation */                        \
    else                                                                       \
      return a.func##_(); /* Forward to array */                               \
  }

/// Binary operation router - dispatches to member function
#define EL_ROUTE_BINARY(name, func)                                            \
  template <typename T1, typename T2>                                          \
    requires(is_array_v<T1> || is_array_v<T2>)                                 \
  EL_INLINE auto name(const T1& a1, const T2& a2)                              \
  {                                                                            \
    using E = expr_t<T1, T2>;                                                  \
    if constexpr (std::is_same_v<T1, E> && std::is_same_v<T2, E>)              \
      return a1.derived().func##_(a2.derived());                               \
    else                                                                       \
      return name(static_cast<ref_cast_t<T1, E>>(a1),                          \
                  static_cast<ref_cast_t<T2, E>>(a2));                         \
  }

/// Binary operation router with scalar fallback
#define EL_ROUTE_BINARY_FALLBACK(name, func, expr)                             \
  template <typename T1, typename T2>                                          \
  EL_INLINE auto name(const T1& a1, const T2& a2)                              \
  {                                                                            \
    using E = expr_t<T1, T2>;                                                  \
    if constexpr (is_array_any_v<T1, T2>) {                                    \
      if constexpr (std::is_same_v<T1, E> && std::is_same_v<T2, E>)            \
        return a1.derived().func##_(a2.derived());                             \
      else                                                                     \
        return name(static_cast<ref_cast_t<T1, E>>(a1),                        \
                    static_cast<ref_cast_t<T2, E>>(a2));                       \
    }                                                                          \
    else {                                                                     \
      return expr;                                                             \
    }                                                                          \
  }

/// Ternary operation router with scalar fallback
#define EL_ROUTE_TERNARY_FALLBACK(name, func, expr)                            \
  template <typename T1, typename T2, typename T3>                             \
  EL_INLINE auto name(const T1& a1, const T2& a2, const T3& a3)                \
  {                                                                            \
    using E = expr_t<T1, T2, T3>;                                              \
    if constexpr (is_array_any_v<T1, T2, T3>) {                                \
      if constexpr (std::is_same_v<T1, E> && std::is_same_v<T2, E> &&          \
                    std::is_same_v<T3, E>)                                     \
        return a1.derived().func##_(a2.derived(), a3.derived());               \
      else                                                                     \
        return name(static_cast<ref_cast_t<T1, E>>(a1),                        \
                    static_cast<ref_cast_t<T2, E>>(a2),                        \
                    static_cast<ref_cast_t<T3, E>>(a3));                       \
    }                                                                          \
    else {                                                                     \
      return expr;                                                             \
    }                                                                          \
  }

// -----------------------------------------------------------------------------
// Forward declarations for scalar fallback functions (defined in
// arrayutils.hpp)
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN(detail)
template <typename T> T rcp_(const T& a);
template <typename T> T rsqrt_(const T& a);
template <typename T> T sqrt_(const T& a);
template <typename T> T fmadd_(const T& a, const T& b, const T& c);
template <typename T> T abs_(const T& a);
template <typename T> T floor_(const T& a);
template <typename T> T ceil_(const T& a);
template <typename T> T round_(const T& a);
template <typename T> T trunc_(const T& a);
template <typename T> T minimum_(const T& a, const T& b);
template <typename T> T maximum_(const T& a, const T& b);
NAMESPACE_END(detail)

// -----------------------------------------------------------------------------
// Arithmetic operators
// -----------------------------------------------------------------------------
EL_ROUTE_BINARY(operator+, add)
EL_ROUTE_BINARY(operator-, sub)
EL_ROUTE_BINARY(operator*, mul)
EL_ROUTE_UNARY(operator-, neg)

/// Division: uses reciprocal approximation when dividing by a lower-depth
/// floating-point value (e.g. array / scalar)
template <typename T1, typename T2>
  requires(is_array_v<T1> || is_array_v<T2>)
EL_INLINE auto operator/(const T1& a1, const T2& a2)
{
  using E  = expr_t<T1, T2>;
  using E2 = expr_t<scalar_t<T1>, T2>;

  if constexpr (std::is_same_v<T1, E> && std::is_same_v<T2, E>)
    return a1.derived().div_(a2.derived());
  else if constexpr (is_floating_point_v<scalar_t<E>> &&
                     depth_v<T1> > depth_v<T2>)
    return static_cast<ref_cast_t<T1, E>>(a1) *
           rcp(static_cast<ref_cast_t<T1, E2>>(a2));
  else
    return operator/(static_cast<ref_cast_t<T1, E>>(a1),
                     static_cast<ref_cast_t<T2, E>>(a2));
}

/// Compound assignment operators
#define EL_ROUTE_COMPOUND_OPERATOR(op)                                         \
  template <typename T1, typename T2>                                          \
    requires(is_array_v<T1> && !std::is_const_v<T1>)                           \
  EL_INLINE T1& operator op##=(T1& a1, const T2& a2)                          \
  {                                                                            \
    a1 = a1 op a2;                                                             \
    return a1;                                                                 \
  }

EL_ROUTE_COMPOUND_OPERATOR(+)
EL_ROUTE_COMPOUND_OPERATOR(-)
EL_ROUTE_COMPOUND_OPERATOR(*)
EL_ROUTE_COMPOUND_OPERATOR(/)

#undef EL_ROUTE_COMPOUND_OPERATOR

// -----------------------------------------------------------------------------
// Comparison operators
// -----------------------------------------------------------------------------
EL_ROUTE_BINARY(operator==, eq)
EL_ROUTE_BINARY(operator!=, neq)
EL_ROUTE_BINARY(operator<, lt)
EL_ROUTE_BINARY(operator<=, le)
EL_ROUTE_BINARY(operator>, gt)
EL_ROUTE_BINARY(operator>=, ge)

// -----------------------------------------------------------------------------
// Basic math function routers
// Only fundamental operations are routed here; trig/transcendentals stay in
// math.hpp
// -----------------------------------------------------------------------------
EL_ROUTE_UNARY_FALLBACK(sqrt, sqrt, detail::sqrt_(a))
EL_ROUTE_UNARY_FALLBACK(rcp, rcp, detail::rcp_(a))
EL_ROUTE_UNARY_FALLBACK(rsqrt, rsqrt, detail::rsqrt_(a))
EL_ROUTE_UNARY_FALLBACK(abs, abs, detail::abs_(a))
EL_ROUTE_UNARY_FALLBACK(floor, floor, detail::floor_(a))
EL_ROUTE_UNARY_FALLBACK(ceil, ceil, detail::ceil_(a))
EL_ROUTE_UNARY_FALLBACK(round, round, detail::round_(a))
EL_ROUTE_UNARY_FALLBACK(trunc, trunc, detail::trunc_(a))
EL_ROUTE_BINARY_FALLBACK(minimum, minimum, detail::minimum_((E) a1, (E) a2))
EL_ROUTE_BINARY_FALLBACK(maximum, maximum, detail::maximum_((E) a1, (E) a2))

// -----------------------------------------------------------------------------
// Horizontal operations
// These dispatch to member functions for SIMD specialization
// -----------------------------------------------------------------------------
EL_ROUTE_UNARY_FALLBACK(all, all, a)
EL_ROUTE_UNARY_FALLBACK(any, any, a)
EL_ROUTE_UNARY_FALLBACK(hsum, sum, a)
EL_ROUTE_UNARY_FALLBACK(hprod, prod, a)
EL_ROUTE_UNARY_FALLBACK(hmin, min, a)
EL_ROUTE_UNARY_FALLBACK(hmax, max, a)
EL_ROUTE_BINARY_FALLBACK(dot, dot, (E) a1*(E) a2)

// -----------------------------------------------------------------------------
// Ternary operations
// -----------------------------------------------------------------------------
EL_ROUTE_TERNARY_FALLBACK(fmadd, fmadd, detail::fmadd_((E) a1, (E) a2, (E) a3))
EL_ROUTE_TERNARY_FALLBACK(fmsub, fmsub, detail::fmadd_((E) a1, (E) a2, -(E) a3))
EL_ROUTE_TERNARY_FALLBACK(fnmadd,
                          fnmadd,
                          detail::fmadd_(-(E) a1, (E) a2, (E) a3))
EL_ROUTE_TERNARY_FALLBACK(fnmsub,
                          fnmsub,
                          detail::fmadd_(-(E) a1, (E) a2, -(E) a3))

#undef EL_ROUTE_UNARY
#undef EL_ROUTE_UNARY_FALLBACK
#undef EL_ROUTE_BINARY_FALLBACK
#undef EL_ROUTE_TERNARY_FALLBACK

// -----------------------------------------------------------------------------
// Shuffle operation
// -----------------------------------------------------------------------------

/// Shuffle the entries of an array according to compile-time indices
template <size_t... Indices, typename T> EL_INLINE auto shuffle(const T& a)
{
  if constexpr (is_array_v<T>) {
    return a.template shuffle_<Indices...>();
  }
  else {
    // Scalar case: only allow single-element shuffle with index 0
    static_assert(sizeof...(Indices) == 1 && (... && (Indices == 0)),
                  "shuffle(): scalar argument requires single index 0!");
    return a;
  }
}

// -----------------------------------------------------------------------------
// Utility functions (clamp, lerp)
// -----------------------------------------------------------------------------

/// Clamp value between lo and hi
template <typename T, typename Lo, typename Hi>
EL_INLINE auto clamp(const T& val, const Lo& lo, const Hi& hi)
{
  return minimum(maximum(val, lo), hi);
}

/// Linear interpolation: a + (b - a) * t
template <typename T, typename U>
EL_INLINE auto lerp(const T& a, const T& b, const U& t)
{
  return fmadd(b - a, t, a);
}

// -----------------------------------------------------------------------------
// Vector operations
// These are standalone implementations that compose primitives
// -----------------------------------------------------------------------------

/// Squared norm (length squared)
template <typename T> EL_INLINE auto squaredNorm(const T& v)
{
  if constexpr (depth_v<T> == 1 || size_v<T> == 0) {
    return hsum(v * v);
  }
  else {
    value_t<T> result = square(v.x());
    for (size_t i = 1; i < size_v<T>; ++i)
      result = fmadd(v.entry(i), v.entry(i), result);
    return result;
  }
}

/// Norm (length)
template <typename T> EL_INLINE auto norm(const T& v)
{
  return sqrt(squaredNorm(v));
}

/// Normalize vector
template <typename T> EL_INLINE auto normalize(const T& v)
{
  return v * rsqrt(squaredNorm(v));
}

/// Cross product (3D only)
/// Uses shuffle operations for SIMD efficiency on both scalar and vectorized
/// types
template <typename T1, typename T2>
EL_INLINE auto cross(const T1& v1, const T2& v2)
{
  static_assert(size_v<T1> == 3 && size_v<T2> == 3,
                "cross(): requires 3D input arrays!");

#if defined(__ARM_ARCH) || defined(__aarch64__) || defined(_M_ARM) ||          \
  defined(_M_ARM64)
  // ARM architecture: use fnmadd (c - a*b)
  return fnmadd(shuffle<2, 0, 1>(v1),
                shuffle<1, 2, 0>(v2),
                shuffle<1, 2, 0>(v1) * shuffle<2, 0, 1>(v2));
#else
  // x86 and other architectures: use fmsub (a*b - c)
  return fmsub(shuffle<1, 2, 0>(v1),
               shuffle<2, 0, 1>(v2),
               shuffle<2, 0, 1>(v1) * shuffle<1, 2, 0>(v2));
#endif
}

// -----------------------------------------------------------------------------
// Array concatenation
// -----------------------------------------------------------------------------

/// Concatenate two arrays (or an array and a scalar) into a larger array
template <typename T1, typename T2>
  requires(is_array_any_v<T1, T2>)
EL_INLINE auto concat(const T1& a1, const T2& a2)
{
  constexpr size_t Size1 = size_v<T1>;
  constexpr size_t Size2 = size_v<T2>;

  static_assert(std::is_same_v<scalar_t<T1>, scalar_t<T2>>,
                "concat(): scalar types must be identical!");

  using Result = Array<value_t<expr_t<T1, T2>>, Size1 + Size2>;
  Result result;

  if constexpr (is_array_v<T1>) {
    for (size_t i = 0; i < Size1; ++i)
      result.entry(i) = a1.derived().entry(i);
  }
  else {
    result.entry(0) = a1;
  }

  if constexpr (is_array_v<T2>) {
    for (size_t i = 0; i < Size2; ++i)
      result.entry(i + Size1) = a2.derived().entry(i);
  }
  else {
    result.entry(Size1) = a2;
  }

  return result;
}

// -----------------------------------------------------------------------------
// Zero-initialization
// -----------------------------------------------------------------------------

/// Create a zero-initialized value. For arrays, broadcasts scalar zero.
/// For scalars, returns T(0).
/// TODO: add is_traversable_v branch for struct-level zero-initialization
template <typename T> EL_INLINE T zeros()
{
  if constexpr (is_array_v<T>)
    return T(scalar_t<T>(0));
  else
    return T(0);
}

// -----------------------------------------------------------------------------
// Head / Tail extraction
// -----------------------------------------------------------------------------

namespace detail {
template <typename Result, size_t Offset, typename T, size_t... Index>
EL_INLINE Result extract(const T& a, std::index_sequence<Index...>)
{
  return Result(a.entry(Index + Offset)...);
}
} // namespace detail

/// Extract the first Size elements from an array
template <size_t Size, typename T>
  requires(is_array_v<T>)
EL_INLINE Array<value_t<T>, Size> head(const T& a)
{
  static_assert(Size <= size_v<T>, "head(): Size exceeds array size");
  if constexpr (size_v<T> == Size)
    return a;
  else
    return detail::extract<Array<value_t<T>, Size>, 0>(
      a, std::make_index_sequence<Size>());
}

/// Extract the last Size elements from an array
template <size_t Size, typename T>
  requires(is_array_v<T>)
EL_INLINE Array<value_t<T>, Size> tail(const T& a)
{
  static_assert(Size <= size_v<T>, "tail(): Size exceeds array size");
  if constexpr (size_v<T> == Size)
    return a;
  else
    return detail::extract<Array<value_t<T>, Size>, size_v<T> - Size>(
      a, std::make_index_sequence<Size>());
}

// -----------------------------------------------------------------------------
// Array hashing
// -----------------------------------------------------------------------------
/// Hash an array by combining the hashes of its entries
template <typename T>
  requires is_array_v<T>
EL_INLINE size_t arrayHash(const T& a)
{
  size_t value = 0;
  for (size_t i = 0; i < size_v<T>; ++i) {
    size_t h = std::hash<value_t<T>>{}(a.entry(i));
    value = h ^ (value + 0x9e3779b9 + (h << 6) + (h >> 2));
  }
  return value;
}

NAMESPACE_END(eldr::embr)

/// Macro to specialize std::hash for an array type
#define EL_ARRAY_HASH(Type)                                                    \
  template <typename V, size_t S> struct std::hash<Type<V, S>> {               \
    size_t operator()(const Type<V, S>& a) const                               \
    {                                                                          \
      return eldr::embr::arrayHash(a);                                         \
    }                                                                          \
  };

EL_ARRAY_HASH(eldr::embr::Array)
