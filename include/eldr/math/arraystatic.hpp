// #pragma once
// #include <eldr/math/arraybase.hpp>
//
// namespace eldr::em {
//
// template <typename Value_, size_t Size_, typename Derived_>
// struct StaticArrayT : ArrayBaseT<Value_, Derived_> {
//   using Base = ArrayBaseT<Value_, Derived_>;
//   EL_ARRAY_IMPORT(StaticArrayT, Base);
//
//   using typename Base::Derived;
//   using typename Base::Scalar;
//   using typename Base::Value;
//
//   using Base::derived;
//
//   static constexpr size_t Size = Size_;
//
//   EL_INLINE constexpr size_t size() const { return Derived::Size; }
//
//   EL_INLINE decltype(auto) x() const
//   {
//     static_assert(Derived::ActualSize >= 1,
//                   "StaticArrayBase::x(): requires Size >= 1");
//     return derived().entry(0);
//   }
//
//   EL_INLINE decltype(auto) x()
//   {
//     static_assert(Derived::ActualSize >= 1,
//                   "StaticArrayBase::x(): requires Size >= 1");
//     return derived().entry(0);
//   }
//
//   EL_INLINE decltype(auto) y() const
//   {
//     static_assert(Derived::ActualSize >= 2,
//                   "StaticArrayBase::y(): requires Size >= 2");
//     return derived().entry(1);
//   }
//
//   EL_INLINE decltype(auto) y()
//   {
//     static_assert(Derived::ActualSize >= 2,
//                   "StaticArrayBase::y(): requires Size >= 2");
//     return derived().entry(1);
//   }
//
//   EL_INLINE decltype(auto) z() const
//   {
//     static_assert(Derived::ActualSize >= 3,
//                   "StaticArrayBase::z(): requires Size >= 3");
//     return derived().entry(2);
//   }
//
//   EL_INLINE decltype(auto) z()
//   {
//     static_assert(Derived::ActualSize >= 3,
//                   "StaticArrayBase::z(): requires Size >= 3");
//     return derived().entry(2);
//   }
//
//   EL_INLINE decltype(auto) w() const
//   {
//     static_assert(Derived::ActualSize >= 4,
//                   "StaticArrayBase::w(): requires Size >= 4");
//     return derived().entry(3);
//   }
//
//   EL_INLINE decltype(auto) w()
//   {
//     static_assert(Derived::ActualSize >= 4,
//                   "StaticArrayBase::w(): requires Size >= 4");
//     return derived().entry(3);
//   }
// };
// } // namespace eldr::em
