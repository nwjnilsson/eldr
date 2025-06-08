#pragma once
#include <eldr/math/arraytraits.hpp>
#include <eldr/math/vector.hpp>

namespace eldr::em {

template <typename T, size_t Size, typename... Args>
concept GlmMatConstructible =
  std::constructible_from<glm::mat<Size, Size, T>, Args...>;

template <typename Value_, size_t Size_>
struct Matrix : public glm::mat<Size_, Size_, Value_> {
  using Base  = glm::mat<Size_, Size_, Value_>;
  using Entry = Value_;
  // EL_ARRAY_DEFAULTS(Matrix)
  // EL_ARRAY_IMPORT(Matrix, Base)
  static constexpr bool   IsMatrix{ true };
  static constexpr size_t Size = Size_;

  constexpr Matrix() = default;
  template <typename... Args>
    requires GlmMatConstructible<Value_, Size_>
  constexpr Matrix(Args&&... args) : Base(args...)
  {
  }
};

template <typename T>
  requires is_matrix_v<T>
T identity()
{
  return glm::identity<T>();
}

template <typename T>
  requires is_matrix_v<T>
constexpr T lookAt(const Point<typename T::Entry, 3>&  eye,
                   const Point<typename T::Entry, 3>&  center,
                   const Vector<typename T::Entry, 3>& up)
{
  return glm::lookAt<T>(eye, center, up);
}

template <typename T>
  requires is_matrix_v<T>
constexpr T rotate(const Vector<typename T::Entry, 3>& axis,
                   const typename T::Entry&            angle)
{
  return glm::rotate<T>(T{ 1.0f }, angle, axis);
}

} // namespace eldr::em
