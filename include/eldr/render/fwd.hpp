#pragma once

#include <eldr/core/fwd.hpp>

NAMESPACE_BEGIN(eldr)
// struct BSDFContext;
// class BSDF;
// class OptixDenoiser;
// class Emitter;
// class Endpoint;
// class Film;
// class ImageBlock;
// class Integrator;
// class SamplingIntegrator;
// class MonteCarloIntegrator;
// class AdjointIntegrator;
EL_VARIANT class Medium;
EL_VARIANT class Mesh;
enum class MaterialType : uint8_t;
// class MicrofacetDistribution;
// class ReconstructionFilter;
// class Sampler;
class SceneBase;
EL_VARIANT class Scene;
// class Sensor;
// class PhaseFunction;
// class ProjectiveCamera;
EL_VARIANT class Shape;
// class ShapeGroup;
// class ShapeKDTree;
// class Texture;
// class Volume;
// class VolumeGrid;
// class MeshAttribute;
//
// struct DirectionSample;
// struct PositionSample;
// struct BSDFSample3;
// struct SilhouetteSample;
// struct PhaseFunctionContext;
// struct Interaction;
// struct MediumInteraction;
// struct SurfaceInteraction;
// struct PreliminaryIntersection;

template <typename Float_, typename Spectrum_> struct RenderAliases {
  using Float    = Float_;
  using Spectrum = Spectrum_;

  // Until I use vector types (if ever)
  using FloatU    = Float;
  using SpectrumU = Spectrum;

  // Keeping the spectrum/wavelength related stuff if I want to get into
  // spectral rendering later on

  // using Wavelength          = wavelength_t<Spectrum>;
  // using UnpolarizedSpectrum = unpolarized_spectrum_t<Spectrum>;

  // using StokesVector4f  = StokesVector<UnpolarizedSpectrum>;
  // using MuellerMatrix4f = MuellerMatrix<UnpolarizedSpectrum>;

  using Ray3f = Ray<Point<Float, 3>, Spectrum>;
  // using RayDifferential3f = RayDifferential<Point<3, Float> /*,
  // Spectrum*/>;

  // using PositionSample3f   = PositionSample<Float /*, Spectrum*/>;
  // using DirectionSample3f  = DirectionSample<Float /*, Spectrum*/>;
  // using BSDFSample3f       = BSDFSample3<Float /*, Spectrum*/>;
  // using SilhouetteSample3f = SilhouetteSample<Float /*, Spectrum*/>;
  // using PhaseFunctionContext =
  //   PhaseFunctionContext<Float /*, Spectrum*/>;
  // using Interaction3f        = Interaction<Float /*, Spectrum*/>;
  // using MediumInteraction3f  = MediumInteraction<Float /*, Spectrum*/>;
  // using SurfaceInteraction3f = SurfaceInteraction<Float /*, Spectrum*/>;
  // using PreliminaryIntersection3f =
  //   PreliminaryIntersection<Float, Shape<FloatU /*,
  //   SpectrumU*/>>;

  // using Scene = Scene<FloatU, Spectrum>;
  //  using Sampler = Sampler<FloatU , SpectrumU>;
  //   using MicrofacetDistribution =
  //     MicrofacetDistribution<FloatU , SpectrumU>;
  using Shape = eldr::Shape<FloatU, Spectrum>;
  //  using ShapeGroup  = ShapeGroup<FloatU , SpectrumU>;
  //  using ShapeKDTree = ShapeKDTree<FloatU , SpectrumU>;
  using Mesh = eldr::Mesh<FloatU, SpectrumU>;
  //  using Integrator  = Integrator<FloatU , SpectrumU>;
  //  using SamplingIntegrator =
  //    SamplingIntegrator<FloatU , SpectrumU>;
  //  using MonteCarloIntegrator =
  //    MonteCarloIntegrator<FloatU , SpectrumU>;
  //  using AdjointIntegrator =
  //    AdjointIntegrator<FloatU , SpectrumU>;
  //  using BSDF          = BSDF<FloatU , SpectrumU>;
  //  using OptixDenoiser = OptixDenoiser<FloatU , SpectrumU>;
  //  using Sensor        = Sensor<FloatU , SpectrumU>;
  //  using ProjectiveCamera =
  //    ProjectiveCamera<FloatU , SpectrumU>;
  //  using Emitter       = Emitter<FloatU , SpectrumU>;
  //  using Endpoint      = Endpoint<FloatU , SpectrumU>;
  using Medium = eldr::Medium<FloatU, SpectrumU>;
  //  using PhaseFunction = PhaseFunction<FloatU , SpectrumU>;
  //  using Film          = Film<FloatU , SpectrumU>;
  //  using ImageBlock    = ImageBlock<FloatU , SpectrumU>;
  //  using ReconstructionFilter =
  //    ReconstructionFilter<FloatU , SpectrumU>;
  //  using Texture    = Texture<FloatU , SpectrumU>;
  //  using Volume     = Volume<FloatU , SpectrumU>;
  //  using VolumeGrid = VolumeGrid<FloatU , SpectrumU>;
};

NAMESPACE_END(eldr)

#define EL_IMPORT_BASE(Name, ...)                                              \
  using Base = Name<Float, Spectrum>;                                          \
  EL_USING_MEMBERS(__VA_ARGS__)

#define EL_IMPORT_TYPES_MACRO(x) using x = typename RenderAliases::x;

#define EL_IMPORT_RENDER_BASIC_TYPES()                                         \
  EL_IMPORT_CORE_TYPES()                                                       \
  using RenderAliases = eldr::RenderAliases<Float, Spectrum>;                  \
  using Ray3f         = typename RenderAliases::Ray3f;
// using Wavelength          = typename RenderAliases::Wavelength;
// using UnpolarizedSpectrum = typename RenderAliases::UnpolarizedSpectrum;
// using StokesVector4f      = typename RenderAliases::StokesVector4f;
// using MuellerMatrix4f     = typename RenderAliases::MuellerMatrix4f;
// using RayDifferential3f   = typename RenderAliases::RayDifferential3f;

#define EL_IMPORT_TYPES(...)                                                   \
  EL_IMPORT_RENDER_BASIC_TYPES()                                               \
  EL_MAP(EL_IMPORT_TYPES_MACRO, __VA_ARGS__)
