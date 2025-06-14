#pragma once

#include <eldr/core/fwd.hpp>

/// List of enabled Eldr variants
#define EL_VARIANTS "scalar_rgb, scalar_spectral\n"

/// Default variant to be used by the "eldr" executable
#define EL_DEFAULT_VARIANT "scalar_rgb"

/// Explicitly instantiate all variants of a "struct" template
#define EL_INSTANTIATE_STRUCT(Name)                                            \
  template struct Name<float, Color<float, 3>>;                          \
  template struct Name<float, Spectrum<float, 4>>;

/// Explicitly instantiate all variants of a "class" template
#define EL_INSTANTIATE_CLASS(Name)                                             \
  template class Name<float, Color<float, 3>>;                           \
  template class Name<float, Spectrum<float, 4>>;

/// Call the variant function "func" for a specific variant "variant"
#define EL_INVOKE_VARIANT(variant, func, ...)                                  \
  [&]() {                                                                      \
    if (variant == "scalar_rgb")                                               \
      return func<float, Color<float, 3>>(__VA_ARGS__);                        \
    else if (variant == "scalar_spectral")                                     \
      return func<float, Spectrum<float, 4>>(__VA_ARGS__);                     \
    else                                                                       \
      Throw("Unsupported variant: \"%s\". Must be one of scalar_rgb, "         \
            "scalar_spectral!",                                                \
            variant);                                                          \
  }()

NAMESPACE_BEGIN(eldr)
NAMESPACE_BEGIN(detail)
/// Convert a <Float, Spectrum> type pair into one of the strings in EL_VARIANTS
template <typename Float_, typename Spectrum_>
constexpr const char* get_variant()
{
  if constexpr (std::is_same_v<Float_, float> &&
                std::is_same_v<Spectrum_, Color<float, 3>>)
    return "scalar_rgb";
  else if constexpr (std::is_same_v<Float_, float> &&
                     std::is_same_v<Spectrum_, Spectrum<float, 4>>)
    return "scalar_spectral";
  else
    return "";
}
NAMESPACE_END(detail)
NAMESPACE_END(eldr)
