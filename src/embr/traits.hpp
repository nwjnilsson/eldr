#pragma once
#include <eldr.hpp>
#include <type_traits>

NAMESPACE_BEGIN(eldr::embr)
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
  sizeof(T0) == sizeof(T1) and
  detail::is_floating_point_v<T0> == detail::is_floating_point_v<T1> and
  detail::is_signed_v<T0> == detail::is_signed_v<T1> and
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
constexpr bool is_int32_t_v{ (sizeof(T) == 4 and
                              detail::is_integral_ext_v<T>) };

/// True for any type that can reasonably be packed into a 64 bit integer array
template <typename T>
constexpr bool is_int64_t_v = { sizeof(T) == 8 and
                                detail::is_integral_ext_v<T> };

template <typename... Ts>
using identity_t = typename detail::identity<Ts...>::type;

template <template <typename> class Op, typename Arg>
constexpr bool is_detected_v =
  detail::detector<void, Op, std::decay_t<Arg>>::value;

// constexpr size_t Dynamic = (size_t) -1;

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

template <typename Source, typename Target>
using ref_cast_t =
  std::conditional_t<std::is_same_v<Source, Target>, const Target&, Target>;

template <typename Source, typename Target>
concept decay_same_as = std::is_same_v<std::decay_t<Source>, Target>;

template <typename Source>
using preserve_move_t =
  std::conditional_t<std::is_reference_v<Source>, Source, Source&&>;

template <typename Source, typename Target>
using move_cast_t = std::
  conditional_t<decay_same_as<Source, Target>, preserve_move_t<Source>, Target>;

NAMESPACE_BEGIN(detail)
template <size_t Size> struct sized_types {};

template <> struct sized_types<1> {
  using Int  = int8_t;
  using UInt = uint8_t;
};

template <> struct sized_types<2> {
  using Int  = int16_t;
  using UInt = uint16_t;
};

template <> struct sized_types<4> {
  using Int   = int32_t;
  using UInt  = uint32_t;
  using Float = float;
};

template <> struct sized_types<8> {
  using Int   = int64_t;
  using UInt  = uint64_t;
  using Float = double;
};

template <typename T, typename Value, typename = int> struct replace_scalar {
  using type = Value;
};

template <typename T, typename Value>
  requires is_array_v<T>
struct replace_scalar<T, Value> {
  using Entry = typename replace_scalar<value_t<T>, Value>::type;
  using type  = typename std::decay_t<T>::Derived::template ReplaceValue<Entry>;
};

template <typename T, typename Value, typename = int> struct replace_value {
  using type = Value;
};

template <typename T, typename Value>
  requires is_array_v<T>
struct replace_value<T, Value> {
  using type = typename std::decay_t<T>::Derived::template ReplaceValue<Value>;
};
NAMESPACE_END(detail)

template <typename T, typename Value>
using replace_scalar_t = typename detail::replace_scalar<T, Value>::type;

template <typename T, typename Value>
using replace_value_t = typename detail::replace_value<T, Value>::type;

template <typename T>
using int_array_t =
  replace_scalar_t<T, typename detail::sized_types<sizeof(scalar_t<T>)>::Int>;

template <typename T>
using uint_array_t =
  replace_scalar_t<T, typename detail::sized_types<sizeof(scalar_t<T>)>::UInt>;

template <typename T>
using float_array_t =
  replace_scalar_t<T, typename detail::sized_types<sizeof(scalar_t<T>)>::Float>;

template <typename T> using int8_array_t   = replace_scalar_t<T, int8_t>;
template <typename T> using uint8_array_t  = replace_scalar_t<T, uint8_t>;
template <typename T> using int16_array_t  = replace_scalar_t<T, int16_t>;
template <typename T> using uint16_array_t = replace_scalar_t<T, uint16_t>;
template <typename T> using int32_array_t  = replace_scalar_t<T, int32_t>;
template <typename T> using uint32_array_t = replace_scalar_t<T, uint32_t>;
template <typename T> using int64_array_t  = replace_scalar_t<T, int64_t>;
template <typename T> using uint64_array_t = replace_scalar_t<T, uint64_t>;
// TODO: C++23 will provide float32_t and float64_t in <stdfloat>
template <typename T> using float32_array_t = replace_scalar_t<T, float>;
template <typename T> using float64_array_t = replace_scalar_t<T, double>;
template <typename T> using bool_array_t    = replace_scalar_t<T, bool>;
template <typename T> using size_array_t    = replace_scalar_t<T, size_t>;

NAMESPACE_END(eldr::embr)
