#pragma once
#include <eldr/arrays/arraygeneric.hpp>
#include <eldr/eldr.hpp>
NAMESPACE_BEGIN(eldr)
/// Spectrum base type
template <typename _Val, size_t _size> struct Spectrum : glm::vec<_size, _Val> {
  using Base = glm::vec<_size, _Val>;
  EL_ARRAY_DEFAULTS(Spectrum)
  template <typename... Ts> Spectrum(Ts&&... ts) : Base(std::forward<Ts>(ts)...)
  {
  }
};

/// RGB style color, used for Y, YA, RGB, RGBA
template <typename _Val, size_t _channels>
  requires(_channels <= 4)
struct Color : Spectrum<_Val, _channels> {
  using Base = Spectrum<_Val, _channels>;
  EL_ARRAY_IMPORT(Color, Base)
};

/// SPD represented by a number of coefficients
template <typename _Val, size_t _size>
struct CoefficientSpectrum : Spectrum<_Val, _size> {
  using Base = Spectrum<_Val, _size>;
  EL_ARRAY_IMPORT(CoefficientSpectrum, Base)
};

/// SPD represented by a number of evenly distributed samples
template <typename _Val, size_t _samples>
struct SampledSpectrum : CoefficientSpectrum<_Val, _samples> {
  using Base = CoefficientSpectrum<_Val, _samples>;
  EL_ARRAY_IMPORT(SampledSpectrum, Base)
};

NAMESPACE_END(eldr)
