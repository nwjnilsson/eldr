#pragma once
#include <eldr/math/matrix.hpp>
namespace eldr::em {
template <typename T> EL_INLINE T full(T value, size_t size)
{
  if constexpr (is_array_v<T>)
    return T::Derived::full_(value, size);
  else {
    return T{ value };
  }
}

template <typename T>
  requires(is_array_v<T> and not is_special_v<T>)
EL_INLINE T identity(size_t size)
{
  return full<T>(scalar_t<T>{ 1 }, size);
}

} // namespace eldr::em
