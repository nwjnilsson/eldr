#pragma once
#include <type_traits>

namespace eldr::em {
using size_t = std::size_t;
namespace detail {
/// Identity function for types
template <typename T, typename...> struct identity {
  using type = T;
};

/// Detector pattern that is used to drive many type traits below
template <typename SFINAE, template <typename> typename Op, typename Arg>
struct detector : std::false_type {};

template <template <typename> typename Op, typename Arg>
struct detector<std::void_t<Op<Arg>>, Op, Arg> : std::true_type {};

template <typename...> constexpr bool false_v = false;

template <typename T>
constexpr bool is_integral_ext_v =
  std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>;

template <bool... Args> constexpr bool and_v = (Args and ...);
} // namespace detail

/// True for any type that can reasonably be packed into a 32 bit integer array
template <typename T>
using enable_if_int32_t =
  std::enable_if_t<sizeof(T) == 4 and detail::is_integral_ext_v<T>>;

/// True for any type that can reasonably be packed into a 64 bit integer array
template <typename T>
using enable_if_int64_t =
  std::enable_if_t<sizeof(T) == 8 and detail::is_integral_ext_v<T>>;

template <typename... Ts>
using identity_t = typename detail::identity<Ts...>::type;

template <template <typename> class Op, typename Arg>
constexpr bool is_detected_v =
  detail::detector<void, Op, std::decay_t<Arg>>::value;

constexpr size_t Dynamic = (size_t) -1;

// Type detectors
namespace detail {
template <typename T> using is_array_det = std::enable_if_t<T::IsArr>;
template <typename T>
using is_matrix_det = std::enable_if_t<T::Derived::IsMatrix>;
template <typename T>
using is_vector_det = std::enable_if_t<T::Derived::IsVector>;
template <typename T>
using is_quaternion_det = std::enable_if_t<T::Derived::IsQuaternion>;
template <typename T> using is_special_det = std::enable_if_t<T::IsSpecial>;

template <typename T>
using is_static_array_det =
  std::enable_if_t<T::IsArr and T::Derived::Size != Dynamic>;
template <typename T>
using is_dynamic_array_det =
  std::enable_if_t<T::IsEldr and T::Derived::Size == Dynamic>;

} // namespace detail

template <typename T>
constexpr bool is_array_v = is_detected_v<detail::is_array_det, T>;

template <typename T>
constexpr bool is_static_array_v =
  is_detected_v<detail::is_static_array_det, T>;

template <typename T>
constexpr bool is_matrix_v = is_detected_v<detail::is_matrix_det, T>;

template <typename T>
constexpr bool is_special_v = is_detected_v<detail::is_special_det, T>;

template <typename T>
constexpr bool is_vector_v = is_detected_v<detail::is_vector_det, T>;

template <typename T>
constexpr bool is_quaternion_v = is_detected_v<detail::is_quaternion_det, T>;

namespace detail {
template <typename T, typename = int> struct scalar {
  using type = std::decay_t<T>;
};

template <typename T>
  requires is_array_v<T>
struct scalar<T> {
  using type = typename std::decay_t<T>::Derived::Scalar;
};

template <typename T, typename = int> struct value {
  using type = std::decay_t<T>;
};

template <typename T>
  requires is_array_v<T>
struct value<T> {
  using type = typename std::decay_t<T>::Derived::Value;
};

template <typename T, typename = int> struct array_depth {
  static constexpr size_t value = 0;
};

template <typename T>
  requires is_array_v<T>
struct array_depth<T> {
  static constexpr size_t value = std::decay_t<T>::Derived::Depth;
};

template <typename T, typename = int> struct array_size {
  static constexpr size_t value = 1;
};

template <typename T>
  requires is_array_v<T>
struct array_size<T> {
  static constexpr size_t value = std::decay_t<T>::Derived::Size;
};
} // namespace detail

/// Type trait to access the base scalar type underlying a potentially nested
/// array
template <typename T> using scalar_t = typename detail::scalar<T>::type;

/// Type trait to access the value type of an array
template <typename T> using value_t = typename detail::value<T>::type;

/// Determine the depth of a nested Dr.Jit array (scalars evaluate to zero)
template <typename T> constexpr size_t depth_v = detail::array_depth<T>::value;

/// Determine the size of a nested Dr.Jit array (scalars evaluate to one)
template <typename T> constexpr size_t size_v = detail::array_size<T>::value;

// template <typename T> constexpr bool is_floating_point_v =
// arr::detail::is_floating_point_v<scalar_t<T>> and !is_mask_v<T>; template
// <typename T> constexpr bool is_integral_v =
// arr::detail::is_integral_v<scalar_t<T>> and !is_mask_v<T>; template
// <typename T> constexpr bool is_arithmetic_v =
// arr::detail::is_arithmetic_v<scalar_t<T>> and !is_mask_v<T>; template
// <typename T> constexpr bool is_signed_v =
// arr::detail::is_signed_v<scalar_t<T>>; template <typename T> constexpr bool
// is_unsigned_v = std::is_unsigned_v<scalar_t<T>>; template <typename T>
// constexpr bool is_half_v = std::is_same_v<scalar_t<T>, arr::half> and
// !is_mask_v<T>;

} // namespace eldr::em
