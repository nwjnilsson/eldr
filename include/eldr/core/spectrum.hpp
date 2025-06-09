#pragma once
#include <eldr/math/arraygeneric.hpp>

namespace eldr::core {

template <typename _Val, size_t _Size, typename _Derived>
struct Spectrum
  : em::StaticArray<_Val,
                    _Size,
                    Spectrum<_Val, _Size, Spectrum<_Val, _Size, _Derived>>> {
  using Base = em::StaticArray<_Val, _Size, Spectrum<_Val, _Size, _Derived>>;
  EL_ARRAY_IMPORT(Spectrum, Base)
  using ArrayType = Spectrum;
  using typename Base::Scalar;
};

template <typename _Val, size_t _Size, typename _Derived>
struct CoefficientSpectrum
  : Spectrum<_Val, _Size, CoefficientSpectrum<_Val, _Size, _Derived>> {
  using Base =
    Spectrum<_Val, _Size, CoefficientSpectrum<_Val, _Size, _Derived>>;
  EL_ARRAY_IMPORT(CoefficientSpectrum, Base)
  using typename Base::Scalar;
  using ArrayType = CoefficientSpectrum;
};

template <typename _Val>
struct RgbaSpectrum : CoefficientSpectrum<_Val, 4, RgbaSpectrum<_Val>> {
  using Base = CoefficientSpectrum<_Val, 4, RgbaSpectrum<_Val>>;
  EL_ARRAY_IMPORT(RgbaSpectrum, Base)

  using typename Base::Scalar;
  using ArrayType = RgbaSpectrum;

  decltype(auto) r() const { return Base::x(); }
  decltype(auto) r() { return Base::x(); }

  decltype(auto) g() const { return Base::y(); }
  decltype(auto) g() { return Base::y(); }

  decltype(auto) b() const { return Base::z(); }
  decltype(auto) b() { return Base::z(); }

  decltype(auto) a() const { return Base::w(); }
  decltype(auto) a() { return Base::w(); }
};

template <typename _Val, size_t sample_count_>
struct SampledSpectrum
  : CoefficientSpectrum<_Val,
                        sample_count_,
                        SampledSpectrum<_Val, sample_count_>> {
  using Base = CoefficientSpectrum<_Val,
                                   sample_count_,
                                   SampledSpectrum<_Val, sample_count_>>;
  EL_ARRAY_IMPORT(SampledSpectrum, Base)

  using ArrayType = SampledSpectrum;
  using typename Base::Scalar;
};
} // namespace eldr::core
