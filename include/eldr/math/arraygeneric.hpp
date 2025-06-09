#pragma once
#include <eldr/math/arraystatic.hpp>
// #include <concepts>

namespace eldr::em {

template <typename _Val, size_t _Size, typename _Derived>
struct StaticArray
  : StaticArrayBase<_Val, _Size, StaticArray<_Val, _Size, _Derived>> {
  using Base = StaticArrayBase<_Val, _Size, StaticArray<_Val, _Size, _Derived>>;
  EL_ARRAY_IMPORT(StaticArray, Base)

  using typename Base::Derived;
  using typename Base::Scalar;
  using typename Base::Value;

  using Base::derived;
  using Base::Size;

  EL_INLINE Value&       entry(size_t i) { return data[i]; }
  EL_INLINE const Value& entry(size_t i) const { return data[i]; }

private:
  Value data[Size];
};

} // namespace eldr::em
// #pragma once
// #include <concepts>
// #include <eldr/math/arraystatic.hpp>
//
// #include <eldr/math/glm.hpp>
//
// template <typename T, size_t size, typename... Args>
// concept GlmVecConstructible =
//   std::constructible_from<glm::vec<size, T>, Args...>;
//
// namespace eldr::em {
// template <typename _Val, size_t _Size, typename _Derived>
// struct StaticArray : StaticArrayT<_Val, _Size, _Derived> {
//   using Base = StaticArrayT<_Val, _Size, _Derived>;
//   EL_ARRAY_IMPORT(StaticArray, Base)
//
//   using typename Base::Derived;
//   using typename Base::Scalar;
//   using typename Base::Value;
//
//   using Base::derived;
//   using Base::entry;
//   using Base::size;
//
//   template <typename Value2, typename D2, typename D = _Derived>
//     requires(D::size != D2::size or D::Depth != D2::Depth)
//   StaticArray(const ArrayBaseT<Value2, D2>& v)
//   {
//     if constexpr (D::size == D2::size and D2::BroadcastOuter) {
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
//     for (size_t i = 0; i < _Size; ++i)
//       data[i] = value;
//   }
//
//   template <typename T = _Val>
//     requires(not std::is_same_v<T, Scalar>)
//   StaticArray(const Value& v)
//   {
//     for (size_t i = 0; i < _Size; ++i)
//       data[i] = v;
//   }
//
//   /// Construct from component values
//   template <typename... Ts>
//     requires GlmVecConstructible<Value, size, Ts...>
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
//   glm::vec<_Size, _Val> data;
// };
// } // namespace eldr::em
