
#pragma once

#include <eldr/math/arraygeneric.hpp>
namespace eldr::em {

template <typename Value> Value radians(const Value& a)
{
  // return a * scalar_t<Value>(180.0 / Pi<double>);
  return glm::radians(a);
}

} // namespace eldr::em
