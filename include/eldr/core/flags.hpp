#pragma once
#include <cstdint>
#include <eldr/core/hash.hpp>
#include <type_traits>

namespace eldr {
using FlagRep = uint32_t;
template <typename Type> struct Flags {
  using Rep = FlagRep;
  constexpr explicit Flags() : Flags(Rep{}) {};
  constexpr explicit Flags(Type flag) : Flags(static_cast<Rep>(flag)) {}
  constexpr explicit Flags(Rep flags) : flags_(flags)
  {
    static_assert(std::is_integral_v<Rep>);
    static_assert(std::is_enum_v<Type>);
  };
  Rep flags_;
};
template <typename T> struct is_flag_type : std::false_type {};

template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr size_t hash(Flags<T> flags)
{
  return hash(flags.flags_);
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator|(Flags<T> f1, Flags<T> f2)
{
  return Flags<T>{ f1.flags_ | f2.flags_ };
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator|(T f1, T f2)
{
  return Flags<T>{ static_cast<Flags<T>::Rep>(f1) |
                   static_cast<Flags<T>::Rep>(f2) };
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator|(Flags<T> f1, T f2)
{
  return f1 | Flags<T>{ static_cast<Flags<T>::Rep>(f2) };
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator|=(Flags<T>& f1, T f2)
{
  return f1 = f1 | f2;
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator&(Flags<T> f1, Flags<T> f2)
{
  return Flags<T>{ f1.flags_ & f2.flags_ };
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator&(Flags<T> f1, T f2)
{
  return f1 & Flags<T>{ static_cast<Flags<T>::Rep>(f2) };
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator&=(Flags<T>& f1, T f2)
{
  return f1 = f1 & f2;
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr bool operator==(Flags<T> f1, Flags<T> f2)
{
  return f1.flags_ == f2.flags_;
}
template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator~(T f1)
{
  return Flags<T>{ ~static_cast<Flags<T>::Rep>(f1) };
}

template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr Flags<T> operator+(T f1)
{
  return Flags<T>{ static_cast<Flags<T>::Rep>(f1) };
}

template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr bool operator!(Flags<T> f)
{
  return not f.flags_;
}

template <typename T, std::enable_if<is_flag_type<T>::value, bool> = true>
constexpr auto hasFlag(Flags<T> flags, T flag)
{
  return (flags.flags_ & static_cast<Flags<T>::Rep>(flag)) !=
         typename Flags<T>::Rep{};
}
} // namespace eldr

#define ELDR_DECLARE_FLAG_TYPE(ns, name)                                       \
  using name##Flags = Flags<ns::name>;                                         \
  template <> struct is_flag_type<ns::name> : std::true_type {};
