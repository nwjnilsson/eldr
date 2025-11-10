#pragma once
#include <eldr.hpp>
#include <embr/arraygeneric.hpp>

NAMESPACE_BEGIN(eldr)
/// Spectrum base type
template <typename _Val, size_t _Size>
struct Spectrum : embr::StaticArray<_Val, _Size, Spectrum<_Val, _Size>> {
  using Base = embr::StaticArray<_Val, _Size, Spectrum<_Val, _Size>>;
  EL_ARRAY_DEFAULTS(Spectrum)
  template <typename... Ts> Spectrum(Ts&&... ts) : Base(std::forward<Ts>(ts)...)
  {
  }
};

/// RGB style color, used for Y, YA, RGB, RGBA
template <typename _Val, size_t _Channels>
  requires(_Channels <= 4)
struct Color : Spectrum<_Val, _Channels> {
  using Base = Spectrum<_Val, _Channels>;
  EL_ARRAY_IMPORT(Color, Base)
};

/// SPD represented by a number of coefficients
template <typename _Val, size_t _Size>
struct CoefficientSpectrum : Spectrum<_Val, _Size> {
  using Base = Spectrum<_Val, _Size>;
  EL_ARRAY_IMPORT(CoefficientSpectrum, Base)
};

/// SPD represented by a number of evenly distributed samples
template <typename _Val, size_t _Samples>
struct SampledSpectrum : CoefficientSpectrum<_Val, _Samples> {
  using Base = CoefficientSpectrum<_Val, _Samples>;
  EL_ARRAY_IMPORT(SampledSpectrum, Base)
};

NAMESPACE_END(eldr)
