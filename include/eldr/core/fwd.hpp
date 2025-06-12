#pragma once
#include <eldr/eldr.hpp>
#include <glm/fwd.hpp>

#define EL_VARIANT template <typename Float, typename Spectrum>

using FlagRep = uint32_t;
using byte_t  = std::byte;

NAMESPACE_BEGIN(eldr::core)
class StopWatch;
class Bitmap;
class Struct;
class Stream;
enum LogLevel : int;
class Logger;
class Formatter;
struct ThreadingPolicy;
struct SingleThreaded;
struct MultiThreaded;
class Sink;
class Thread;

// template <typename Point> struct Transform;
// template <typename Value, std::size_t Size_> struct Vector;
// template <typename Value, std::size_t Size_> struct Normal;
template <typename Value, std::size_t Size_> struct Point;
// template <typename Value, std::size_t Size_> struct Matrix;
template <typename Value, size_t Size> struct Spectrum;
template <typename Value, size_t Size> struct CoefficientSpectrum;
template <typename Value, size_t Samples> struct SampledSpectrum;
template <typename Value, size_t Channels>
  requires(Channels <= 4)
struct Color;
template <typename Point, typename Spectrum> struct Ray;

// TODO: experiment with glm SIMD types and compare performance
template <typename Float_> struct Aliases {
  using Float = Float_;

  using Int8   = glm::int8_t;
  using Int32  = glm::int32_t;
  using UInt32 = glm::uint32_t;
  using Int64  = glm::int64_t;
  using UInt64 = glm::uint64_t;
  // using Float16 = glm::float16_t;
  using Float32 = glm::float32_t;
  using Float64 = glm::float64_t;

  using Vector2i = glm::vec<2, Int32>;
  using Vector3i = glm::vec<3, Int32>;
  using Vector4i = glm::vec<4, Int32>;

  using Vector2u = glm::vec<2, UInt32>;
  using Vector3u = glm::vec<3, UInt32>;
  using Vector4u = glm::vec<4, UInt32>;

  using Vector2f = glm::vec<2, Float>;
  using Vector3f = glm::vec<3, Float>;
  using Vector4f = glm::vec<4, Float>;

  using Vector2d = glm::vec<2, Float64>;
  using Vector3d = glm::vec<3, Float64>;
  using Vector4d = glm::vec<4, Float64>;

  using Normal3f = glm::vec<3, Float>;
  using Normal3d = glm::vec<3, Float64>;

  using Point2i = glm::vec<2, Int32>;
  using Point3i = glm::vec<3, Int32>;
  using Point4i = glm::vec<4, Int32>;

  using Point2u = glm::vec<2, UInt32>;
  using Point3u = glm::vec<3, UInt32>;
  using Point4u = glm::vec<4, UInt32>;

  using Point2f = glm::vec<2, Float>;
  using Point3f = glm::vec<3, Float>;
  using Point4f = glm::vec<4, Float>;

  using Point2d = glm::vec<2, Float64>;
  using Point3d = glm::vec<3, Float64>;
  using Point4d = glm::vec<4, Float64>;

  using Matrix2f = glm::mat<2, 2, Float>;
  using Matrix3f = glm::mat<3, 3, Float>;
  using Matrix4f = glm::mat<4, 4, Float>;

  using Matrix2d = glm::mat<2, 2, Float64>;
  using Matrix3d = glm::mat<3, 3, Float64>;
  using Matrix4d = glm::mat<4, 4, Float64>;

  using Quat4f = glm::qua<Float>; // TODO: wrap
  using Quat4d = glm::qua<Float64>;

  using Color1f = Color<Float, 1>;
  using Color3f = Color<Float, 3>;
  using Color4f = Color<Float, 4>;

  using Color1d = Color<Float64, 1>;
  using Color3d = Color<Float64, 3>;
  using Color4d = Color<Float64, 4>;

  using Transform3f = Matrix3f;
  using Transform4f = Matrix4f;
  using Transform3d = Matrix3d;
  using Transform4d = Matrix4d;
};
NAMESPACE_END(eldr::core)

#define EL_IMPORT_CORE_TYPES_PREFIX(Float_, prefix)                            \
  using prefix##Aliases     = eldr::core::Aliases<Float_>;                     \
  using prefix##Vector2i    = typename prefix##Aliases::Vector2i;              \
  using prefix##Vector3i    = typename prefix##Aliases::Vector3i;              \
  using prefix##Vector4i    = typename prefix##Aliases::Vector4i;              \
  using prefix##Vector2u    = typename prefix##Aliases::Vector2u;              \
  using prefix##Vector3u    = typename prefix##Aliases::Vector3u;              \
  using prefix##Vector4u    = typename prefix##Aliases::Vector4u;              \
  using prefix##Vector2f    = typename prefix##Aliases::Vector2f;              \
  using prefix##Vector3f    = typename prefix##Aliases::Vector3f;              \
  using prefix##Vector4f    = typename prefix##Aliases::Vector4f;              \
  using prefix##Vector2d    = typename prefix##Aliases::Vector2d;              \
  using prefix##Vector3d    = typename prefix##Aliases::Vector3d;              \
  using prefix##Vector4d    = typename prefix##Aliases::Vector4d;              \
  using prefix##Normal3f    = typename prefix##Aliases::Normal3f;              \
  using prefix##Normal3d    = typename prefix##Aliases::Normal3d;              \
  using prefix##Point2i     = typename prefix##Aliases::Point2i;               \
  using prefix##Point3i     = typename prefix##Aliases::Point3i;               \
  using prefix##Point4i     = typename prefix##Aliases::Point4i;               \
  using prefix##Point2u     = typename prefix##Aliases::Point2u;               \
  using prefix##Point3u     = typename prefix##Aliases::Point3u;               \
  using prefix##Point4u     = typename prefix##Aliases::Point4u;               \
  using prefix##Point2f     = typename prefix##Aliases::Point2f;               \
  using prefix##Point3f     = typename prefix##Aliases::Point3f;               \
  using prefix##Point4f     = typename prefix##Aliases::Point4f;               \
  using prefix##Point2d     = typename prefix##Aliases::Point2d;               \
  using prefix##Point3d     = typename prefix##Aliases::Point3d;               \
  using prefix##Point4d     = typename prefix##Aliases::Point4d;               \
  using prefix##Matrix2f    = typename prefix##Aliases::Matrix2f;              \
  using prefix##Matrix2d    = typename prefix##Aliases::Matrix2d;              \
  using prefix##Matrix3f    = typename prefix##Aliases::Matrix3f;              \
  using prefix##Matrix3d    = typename prefix##Aliases::Matrix3d;              \
  using prefix##Matrix4f    = typename prefix##Aliases::Matrix4f;              \
  using prefix##Matrix4d    = typename prefix##Aliases::Matrix4d;              \
  using prefix##Quat4f      = typename prefix##Aliases::Quat4f;                \
  using prefix##Quat4d      = typename prefix##Aliases::Quat4d;                \
  using prefix##Color1f     = typename prefix##Aliases::Color1f;               \
  using prefix##Color3f     = typename prefix##Aliases::Color3f;               \
  using prefix##Color4f     = typename prefix##Aliases::Color4f;               \
  using prefix##Color1d     = typename prefix##Aliases::Color1d;               \
  using prefix##Color3d     = typename prefix##Aliases::Color3d;               \
  using prefix##Color4d     = typename prefix##Aliases::Color4d;               \
  using prefix##Transform3f = typename prefix##Aliases::Transform3f;           \
  using prefix##Transform4f = typename prefix##Aliases::Transform4f;           \
  using prefix##Transform3d = typename prefix##Aliases::Transform3d;           \
  using prefix##Transform4d = typename prefix##Aliases::Transform4d;

// Variadic macro to import a set of variables from the base class
#define __EL_USING_TYPES_MACRO__(x) using typename Base::x;
#define EL_USING_TYPES(...) EL_MAP(__EL_USING_TYPES_MACRO__, __VA_ARGS__)

// Variadic macro to import a set of variables from the base class
#define __EL_USING_MEMBERS_MACRO__(x) using Base::x;
#define EL_USING_MEMBERS(...) EL_MAP(__EL_USING_MEMBERS_MACRO__, __VA_ARGS__)

#define EL_IMPORT_CORE_TYPES() EL_IMPORT_CORE_TYPES_PREFIX(Float, )
