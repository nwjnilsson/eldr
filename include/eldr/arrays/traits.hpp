#pragma once
#include <eldr/eldr.hpp>
#include <type_traits>

NAMESPACE_BEGIN(eldr::arr)
NAMESPACE_BEGIN(detail)

template <typename T> struct is_signed : std::is_floating_point<T> {};
template <typename T> constexpr bool is_signed_v{ is_signed<T>::value };
template <typename T> struct is_floating_point : std::is_floating_point<T> {};
template <typename T>
constexpr bool is_floating_point_v{ is_floating_point<T>::value };

template <typename T, typename...> struct identity {
  using type = T;
};

template <typename SFINAE, template <typename> typename Op, typename Arg>
struct detector : std::false_type {};

template <template <typename> typename Op, typename Arg>
struct detector<std::void_t<Op<Arg>>, Op, Arg> : std::true_type {};

template <typename...> constexpr bool false_v = false;

template <typename T>
constexpr bool is_integral_ext_v =
  std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

/// Relaxed type equivalence to work around 'long' vs 'long long' differences
template <typename T0, typename T1>
static constexpr bool is_same_v =
  sizeof(T0) == sizeof(T1) &&
  arr::detail::is_floating_point_v<T0> == detail::is_floating_point_v<T1> &&
  detail::is_signed_v<T0> == detail::is_signed_v<T1> &&
  is_integral_ext_v<T0> == is_integral_ext_v<T1>;

struct reinterpret_flag {};
template <size_t Size, typename... Ts>
constexpr bool is_components_v{ (
  sizeof...(Ts) == Size and Size != 1 and
  (!std::is_same_v<Ts, reinterpret_flag> and ...)) };

template <bool... Args> constexpr bool and_v = (Args && ...);
NAMESPACE_END(detail)

/// True for any type that can reasonably be packed into a 32 bit integer array
template <typename T>
constexpr bool is_int32_t_v{ (sizeof(T) == 4 && detail::is_integral_ext_v<T>) };

/// True for any type that can reasonably be packed into a 64 bit integer array
template <typename T>
constexpr bool is_int64_t_v = { sizeof(T) == 8 &&
                                detail::is_integral_ext_v<T> };

template <typename... Ts>
using identity_t = typename detail::identity<Ts...>::type;

template <template <typename> class Op, typename Arg>
constexpr bool is_detected_v =
  detail::detector<void, Op, std::decay_t<Arg>>::value;

constexpr size_t Dynamic = (size_t) -1;

NAMESPACE_BEGIN(detail)
template <typename T> using is_array_det = std::enable_if_t<T::IsArray>;
NAMESPACE_END(detail)

template <typename T>
constexpr bool is_array_v = is_detected_v<detail::is_array_det, T>;

NAMESPACE_BEGIN(detail)
template <typename T> struct scalar {
  using type = std::decay_t<T>;
};

template <typename T>
  requires is_array_v<T>
struct scalar<T> {
  using type = typename std::decay_t<T>::Derived::Scalar;
};

template <typename T> struct value {
  using type = std::decay_t<T>;
};
template <typename T>
  requires is_array_v<T>
struct value<T> {
  using type = typename std::decay_t<T>::Derived::Scalar;
};

NAMESPACE_END(detail)

template <typename T> using scalar_t = typename detail::scalar<T>::type;
template <typename T> using value_t  = typename detail::value<T>::type;

NAMESPACE_END(eldr::arr)
