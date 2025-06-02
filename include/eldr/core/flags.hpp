#pragma once
#include <cstdint>
#include <eldr/core/hash.hpp>
#include <type_traits>

using FlagRep = uint32_t;

template <typename T> struct is_flag_type : std::false_type {};
template <typename T> constexpr bool is_flag_type_v = is_flag_type<T>::value;

template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
struct Flags {
  using Rep = FlagRep;
  constexpr explicit Flags() : Flags(Rep{}) {};
  constexpr explicit Flags(T flag) : Flags(static_cast<Rep>(flag)) {}
  constexpr explicit Flags(Rep flags) : flags(flags)
  {
    static_assert(std::is_integral_v<Rep>);
    static_assert(std::is_enum_v<T>);
  };
  constexpr bool hasFlag(T flag) const
  {
    return (flags & static_cast<Flags<T>::Rep>(flag)) !=
           typename Flags<T>::Rep{};
  }

  Rep flags;
};

template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr size_t hash(Flags<T> flags)
{
  return std::hash<FlagRep>()(flags.flags);
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator|(Flags<T> f1, Flags<T> f2)
{
  return Flags<T>{ f1.flags | f2.flags };
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator|(T f1, T f2)
{
  return Flags<T>{ f1 } | Flags<T>{ f2 };
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator|(Flags<T> f1, T f2)
{
  return f1 | Flags<T>{ f2 };
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator|=(Flags<T>& f1, T f2)
{
  return f1 = f1 | Flags<T>{ f2 };
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator&(Flags<T> f1, Flags<T> f2)
{
  return Flags<T>{ f1.flags & f2.flags };
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator&(Flags<T> f1, T f2)
{
  return f1 & Flags<T>{ f2 };
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator&=(Flags<T>& f1, T f2)
{
  return f1 = f1 & Flags<T>{ f2 };
}

template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr bool operator==(Flags<T> f1, Flags<T> f2)
{
  return f1.flags == f2.flags;
}
template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator~(T f1)
{
  return Flags<T>{ ~static_cast<Flags<T>::Rep>(f1) };
}

template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr Flags<T> operator+(T f1)
{
  return Flags<T>{ f1 };
}

template <typename T, std::enable_if_t<is_flag_type_v<T>, bool> = true>
constexpr bool operator!(Flags<T> f)
{
  return not f.flags;
}

// This assumes we're in the global namespace
#define ELDR_DECLARE_FLAG_SPEC(ns, name)                                       \
  template <> struct is_flag_type<ns::name> : std::true_type {};               \
  namespace ns {                                                               \
  using name##Flags = Flags<name>;                                             \
  }
