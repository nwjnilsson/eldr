#pragma once

#include <eldr/core/fwd.hpp>

NAMESPACE_BEGIN(eldr)
NAMESPACE_BEGIN(render)
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
NAMESPACE_END(render)

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
  //   render::PhaseFunctionContext<Float /*, Spectrum*/>;
  // using Interaction3f        = Interaction<Float /*, Spectrum*/>;
  // using MediumInteraction3f  = MediumInteraction<Float /*, Spectrum*/>;
  // using SurfaceInteraction3f = SurfaceInteraction<Float /*, Spectrum*/>;
  // using PreliminaryIntersection3f =
  //   PreliminaryIntersection<Float, render::Shape<FloatU /*,
  //   SpectrumU*/>>;

  using Scene = render::Scene<FloatU, Spectrum>;
  // using Sampler = render::Sampler<FloatU , SpectrumU>;
  //  using MicrofacetDistribution =
  //    render::MicrofacetDistribution<FloatU , SpectrumU>;
  using Shape = render::Shape<FloatU, Spectrum>;
  //  using ShapeGroup  = render::ShapeGroup<FloatU , SpectrumU>;
  //  using ShapeKDTree = render::ShapeKDTree<FloatU , SpectrumU>;
  using Mesh = render::Mesh<FloatU, SpectrumU>;
  //  using Integrator  = render::Integrator<FloatU , SpectrumU>;
  //  using SamplingIntegrator =
  //    render::SamplingIntegrator<FloatU , SpectrumU>;
  //  using MonteCarloIntegrator =
  //    render::MonteCarloIntegrator<FloatU , SpectrumU>;
  //  using AdjointIntegrator =
  //    render::AdjointIntegrator<FloatU , SpectrumU>;
  //  using BSDF          = render::BSDF<FloatU , SpectrumU>;
  //  using OptixDenoiser = render::OptixDenoiser<FloatU , SpectrumU>;
  //  using Sensor        = render::Sensor<FloatU , SpectrumU>;
  //  using ProjectiveCamera =
  //    render::ProjectiveCamera<FloatU , SpectrumU>;
  //  using Emitter       = render::Emitter<FloatU , SpectrumU>;
  //  using Endpoint      = render::Endpoint<FloatU , SpectrumU>;
  using Medium = render::Medium<FloatU, SpectrumU>;
  //  using PhaseFunction = render::PhaseFunction<FloatU , SpectrumU>;
  //  using Film          = render::Film<FloatU , SpectrumU>;
  //  using ImageBlock    = render::ImageBlock<FloatU , SpectrumU>;
  //  using ReconstructionFilter =
  //    render::ReconstructionFilter<FloatU , SpectrumU>;
  //  using Texture    = render::Texture<FloatU , SpectrumU>;
  //  using Volume     = render::Volume<FloatU , SpectrumU>;
  //  using VolumeGrid = render::VolumeGrid<FloatU , SpectrumU>;
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
