#pragma once
#include <eldr.hpp>
#include <type_traits>

NAMESPACE_BEGIN(eldr::embr)
NAMESPACE_BEGIN(detail)

// Scalar-level type traits (these work on scalar types only, not arrays)
template <typename T>
constexpr bool is_scalar_v =
  std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

template <typename T>
constexpr bool is_floating_point_v = std::is_floating_point_v<T>;

template <typename T> constexpr bool is_integral_v = std::is_integral_v<T>;

template <typename T> constexpr bool is_arithmetic_v = std::is_arithmetic_v<T>;

template <typename T>
constexpr bool is_signed_v = std::is_signed_v<T> || std::is_floating_point_v<T>;

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
template <size_t size, typename... Ts>
constexpr bool is_components_v{ (
  sizeof...(Ts) == size and size != 1 and
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

// -----------------------------------------------------------------------------
// Type detectors for array properties
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN(detail)
template <typename T> using is_array_det = std::enable_if_t<T::kIsArray>;
template <typename T>
using is_mask_det = std::enable_if_t<T::kIsArray && T::Derived::kIsMask>;
template <typename T>
using is_packed_array_det =
  std::enable_if_t<T::kIsArray && T::Derived::kIsPacked>;
template <typename T>
using is_recursive_array_det =
  std::enable_if_t<T::kIsArray && T::Derived::kIsRecursive>;
template <typename T>
using is_kmask_det = std::enable_if_t<T::kIsArray && T::Derived::kIsKMask>;
template <typename T>
using is_complex_det = std::enable_if_t<T::Derived::kIsComplex>;
template <typename T>
using is_matrix_det = std::enable_if_t<T::Derived::kIsMatrix>;
template <typename T>
using is_vector_det = std::enable_if_t<T::Derived::kIsVector>;
template <typename T>
using is_quaternion_det = std::enable_if_t<T::Derived::kIsQuaternion>;
template <typename T>
using is_tensor_det = std::enable_if_t<T::Derived::kIsTensor>;
template <typename T>
using is_special_det = std::enable_if_t<T::Derived::kIsSpecial>;
template <typename T>
using is_masked_array_det =
  std::enable_if_t<T::kIsArray && T::Derived::kIsMaskedArray>;
NAMESPACE_END(detail)

// -----------------------------------------------------------------------------
// Public type traits using detector pattern
// -----------------------------------------------------------------------------

template <typename T>
constexpr bool is_array_v = is_detected_v<detail::is_array_det, T>;

template <typename T>
constexpr bool is_mask_v =
  std::is_same_v<T, bool> || is_detected_v<detail::is_mask_det, T>;

template <typename T>
constexpr bool is_masked_array_v =
  is_detected_v<detail::is_masked_array_det, T>;

template <typename T>
constexpr bool is_packed_array_v =
  is_detected_v<detail::is_packed_array_det, T>;

template <typename T>
constexpr bool is_recursive_array_v =
  is_detected_v<detail::is_recursive_array_det, T>;

template <typename T>
constexpr bool is_kmask_v = is_detected_v<detail::is_kmask_det, T>;

template <typename... Ts>
constexpr bool is_array_any_v = (is_array_v<Ts> || ...);

template <typename T>
constexpr bool is_complex_v = is_detected_v<detail::is_complex_det, T>;

template <typename T>
constexpr bool is_matrix_v = is_detected_v<detail::is_matrix_det, T>;

template <typename T>
constexpr bool is_vector_v = is_detected_v<detail::is_vector_det, T>;

template <typename T>
constexpr bool is_quaternion_v = is_detected_v<detail::is_quaternion_det, T>;

template <typename T>
constexpr bool is_tensor_v = is_detected_v<detail::is_tensor_det, T>;

template <typename T>
constexpr bool is_special_v = is_detected_v<detail::is_special_det, T>;

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
template <typename T> using value_t = typename detail::value<T>::type;

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
template <size_t size> struct sized_types {};

template <> struct sized_types<1> {
  using Int = int8_t;
  using UInt = uint8_t;
};

template <> struct sized_types<2> {
  using Int = int16_t;
  using UInt = uint16_t;
};

template <> struct sized_types<4> {
  using Int = int32_t;
  using UInt = uint32_t;
  using Float = float;
};

template <> struct sized_types<8> {
  using Int = int64_t;
  using UInt = uint64_t;
  using Float = double;
};

template <typename T, typename Value, typename = int> struct replace_scalar {
  using type = Value;
};

template <typename T, typename Value>
  requires is_array_v<T>
struct replace_scalar<T, Value> {
  using Entry = typename replace_scalar<value_t<T>, Value>::type;
  using type = typename std::decay_t<T>::Derived::template ReplaceValue<Entry>;
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

template <typename T> using int8_array_t = replace_scalar_t<T, int8_t>;
template <typename T> using uint8_array_t = replace_scalar_t<T, uint8_t>;
template <typename T> using int16_array_t = replace_scalar_t<T, int16_t>;
template <typename T> using uint16_array_t = replace_scalar_t<T, uint16_t>;
template <typename T> using int32_array_t = replace_scalar_t<T, int32_t>;
template <typename T> using uint32_array_t = replace_scalar_t<T, uint32_t>;
template <typename T> using int64_array_t = replace_scalar_t<T, int64_t>;
template <typename T> using uint64_array_t = replace_scalar_t<T, uint64_t>;
template <typename T> using float32_array_t = replace_scalar_t<T, float>;
template <typename T> using float64_array_t = replace_scalar_t<T, double>;
template <typename T> using bool_array_t = replace_scalar_t<T, bool>;

template <typename T> using size_array_t = replace_scalar_t<T, size_t>;

// -----------------------------------------------------------------------------
// Array size trait
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN(detail)
template <typename T> struct array_size {
  static constexpr size_t value = 1;
};
template <typename T>
  requires is_array_v<T>
struct array_size<T> {
  static constexpr size_t value = std::decay_t<T>::Derived::kSize;
};
NAMESPACE_END(detail)

template <typename T> constexpr size_t size_v = detail::array_size<T>::value;

// -----------------------------------------------------------------------------
// Array depth trait (nesting level)
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN(detail)
template <typename T> struct array_depth {
  static constexpr size_t value = 0;
};
template <typename T>
  requires is_array_v<T>
struct array_depth<T> {
  static constexpr size_t value = std::decay_t<T>::Derived::kDepth;
};
NAMESPACE_END(detail)

template <typename T> constexpr size_t depth_v = detail::array_depth<T>::value;

// -----------------------------------------------------------------------------
// Expression type trait
// Finds the deepest array type and combines with common scalar type
// -----------------------------------------------------------------------------
NAMESPACE_BEGIN(detail)

/// Find the deepest (most nested) array type from a list
template <typename... Args> struct deepest;
template <> struct deepest<> {
  using type = void;
};
template <typename Arg, typename... Args> struct deepest<Arg, Args...> {
private:
  using T0 = Arg;
  using T1 = typename deepest<Args...>::type;
  static constexpr size_t D0 = depth_v<T0>;
  static constexpr size_t D1 = depth_v<T1>;

public:
  using type = std::conditional_t<(D1 > D0 || D0 == 0), T1, T0>;
};

template <typename... Ts> using deepest_t = typename deepest<Ts...>::type;

/// Compute the common scalar type for arithmetic expressions
template <typename... Ts> struct expr {
  using type = decltype((std::declval<Ts>() + ...));
};
template <typename T> struct expr<T> {
  using type = std::decay_t<T>;
};

NAMESPACE_END(detail)

/// Type trait to compute the type of an arithmetic expression involving Ts...
template <typename... Ts>
using expr_t = replace_scalar_t<detail::deepest_t<Ts...>,
                                typename detail::expr<scalar_t<Ts>...>::type>;

// -----------------------------------------------------------------------------
// Array-aware type traits (work for both scalars and arrays)
// These extract the scalar type and check it
// -----------------------------------------------------------------------------

template <typename T>
constexpr bool is_floating_point_v =
  detail::is_floating_point_v<scalar_t<T>> && !is_mask_v<T>;

template <typename T>
constexpr bool is_integral_v =
  detail::is_integral_v<scalar_t<T>> && !is_mask_v<T>;

template <typename T>
constexpr bool is_arithmetic_v =
  detail::is_arithmetic_v<scalar_t<T>> && !is_mask_v<T>;

template <typename T>
constexpr bool is_signed_v = detail::is_signed_v<scalar_t<T>>;

template <typename T>
constexpr bool is_unsigned_v = std::is_unsigned_v<scalar_t<T>>;



template <typename T> using mask_t = replace_scalar_t<T, bool>;

NAMESPACE_END(eldr::embr)
