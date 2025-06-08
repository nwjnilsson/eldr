#pragma once
#include <eldr/math/arraybase.hpp>
#include <eldr/math/glm.hpp>
// #include <concepts>

namespace eldr::em {
template <typename T, size_t Size, typename... Args>
concept GlmVecConstructible =
  std::constructible_from<glm::vec<Size, T>, Args...>;

template <typename Value_, size_t Size_, typename Derived_>
struct StaticArray : glm::vec<Size_, Value_> {
  using Base = glm::vec<Size_, Value_>;
  EL_ARRAY_IMPORT(StaticArray, Base)

  static constexpr size_t Size = Size_;
  using Derived                = Derived_;
  using Value                  = Value_;
  using Scalar =
    Value; // I won't use vector of vector with glm, Value is same as scalar

  // using typename Base::Derived;
  // using typename Base::Scalar;
  // using typename Base::Value;

  // using Base::derived;
  // using Base::entry;
  // using Base::Size;
};

} // namespace eldr::em
// #pragma once
// #include <concepts>
// #include <eldr/math/arraystatic.hpp>
//
// #include <eldr/math/glm.hpp>
//
// template <typename T, size_t Size, typename... Args>
// concept GlmVecConstructible =
//   std::constructible_from<glm::vec<Size, T>, Args...>;
//
// namespace eldr::em {
// template <typename Value_, size_t Size_, typename Derived_>
// struct StaticArray : StaticArrayT<Value_, Size_, Derived_> {
//   using Base = StaticArrayT<Value_, Size_, Derived_>;
//   EL_ARRAY_IMPORT(StaticArray, Base)
//
//   using typename Base::Derived;
//   using typename Base::Scalar;
//   using typename Base::Value;
//
//   using Base::derived;
//   using Base::entry;
//   using Base::Size;
//
//   template <typename Value2, typename D2, typename D = Derived_>
//     requires(D::Size != D2::Size or D::Depth != D2::Depth)
//   StaticArray(const ArrayBaseT<Value2, D2>& v)
//   {
//     if constexpr (D::Size == D2::Size and D2::BroadcastOuter) {
//       static_assert(std::is_constructible_v<Value, value_t<D2>>);
//       for (size_t i = 0; i < derived().size(); ++i)
//         derived().entry(i) = static_cast<Value>(v.derived().entry(i));
//     }
//     else {
//       static_assert(std::is_constructible_v<Value, D2>);
//       for (size_t i = 0; i < derived().size(); ++i)
//         derived().entry(i) = v.derived();
//     }
//   }
//
//   template <typename T>
//     requires(std::is_scalar_v<T>)
//   StaticArray(T v)
//   {
//     Scalar value = static_cast<Scalar>(v);
//     for (size_t i = 0; i < Size_; ++i)
//       data[i] = value;
//   }
//
//   template <typename T = Value_>
//     requires(not std::is_same_v<T, Scalar>)
//   StaticArray(const Value& v)
//   {
//     for (size_t i = 0; i < Size_; ++i)
//       data[i] = v;
//   }
//
//   /// Construct from component values
//   template <typename... Ts>
//     requires GlmVecConstructible<Value, Size, Ts...>
//   EL_INLINE StaticArray(Ts&&... ts) : data{ move_cast_t<Ts, Value>(ts)... }
//   {
//   }
//
//   /// Construct from sub-arrays
//   // template <typename T1, typename T2, typename T = StaticArray>
//   //   requires(depth_v<T1> == depth_v<T> and size_v<T1> == Base::Size1 and
//   //            depth_v<T2> == depth_v<T> and size_v<T2> == Base::Size2 and
//   //            Base::Size2 != 0)
//   // StaticArray(T1&& a1, T2&& a2)
//   //   : StaticArray(a1,
//   //                 a2,
//   //                 std::make_index_sequence<Base::Size1>(),
//   //                 std::make_index_sequence<Base::Size2>())
//   // {
//   // }
//
//   // bool operator==(const StaticArray other) { return data == other.data; }
//
// private:
//   glm::vec<Size_, Value_> data;
// };
// } // namespace eldr::em
