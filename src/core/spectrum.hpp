#pragma once
#include <eldr.hpp>
#include <embr/array.hpp>

NAMESPACE_BEGIN(eldr)
/// Spectrum base type
template <typename _Val, size_t _Sz>
struct Spectrum : embr::StaticArray<_Val, _Sz, false, Spectrum<_Val, _Sz>> {
  using Base = embr::StaticArray<_Val, _Sz, false, Spectrum<_Val, _Sz>>;
  template <typename T> using ReplaceValue = Spectrum<T, _Sz>;
  EL_ARRAY_IMPORT(Spectrum, Base)
};

/// RGB style color, used for Y, YA, RGB, RGBA
template <typename _Val, size_t _Channels>
  requires(_Channels <= 4)
struct Color : Spectrum<_Val, _Channels> {
  using Base = Spectrum<_Val, _Channels>;
  template <typename T> using ReplaceValue = Color<T, _Channels>;
  EL_ARRAY_IMPORT(Color, Base)
};

/// SPD represented by a number of coefficients
template <typename _Val, size_t _Sz>
struct CoefficientSpectrum : Spectrum<_Val, _Sz> {
  using Base = Spectrum<_Val, _Sz>;
  template <typename T> using ReplaceValue = CoefficientSpectrum<T, _Sz>;
  EL_ARRAY_IMPORT(CoefficientSpectrum, Base)
};

/// SPD represented by a number of evenly distributed samples
template <typename _Val, size_t _Samples>
struct SampledSpectrum : CoefficientSpectrum<_Val, _Samples> {
  using Base = CoefficientSpectrum<_Val, _Samples>;
  template <typename T> using ReplaceValue = SampledSpectrum<T, _Samples>;
  EL_ARRAY_IMPORT(SampledSpectrum, Base)
};

NAMESPACE_END(eldr)

EL_ARRAY_HASH(eldr::Spectrum)
EL_ARRAY_HASH(eldr::Color)
