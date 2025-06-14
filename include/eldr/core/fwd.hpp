#pragma once
#include <eldr/eldr.hpp>
#include <glm/fwd.hpp>

#define EL_VARIANT template <typename Float, typename Spectrum>

using FlagRep = uint32_t;
using byte_t  = std::byte;

NAMESPACE_BEGIN(eldr)
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

template <typename Float_> struct CoreAliases {
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

  using Quat4f = glm::qua<Float>;
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
NAMESPACE_END(eldr)

#define EL_IMPORT_CORE_TYPES_PREFIX(Float_, prefix)                            \
  using prefix##CoreAliases = eldr::CoreAliases<Float_>;                       \
  using prefix##Vector2i    = typename prefix##CoreAliases::Vector2i;          \
  using prefix##Vector3i    = typename prefix##CoreAliases::Vector3i;          \
  using prefix##Vector4i    = typename prefix##CoreAliases::Vector4i;          \
  using prefix##Vector2u    = typename prefix##CoreAliases::Vector2u;          \
  using prefix##Vector3u    = typename prefix##CoreAliases::Vector3u;          \
  using prefix##Vector4u    = typename prefix##CoreAliases::Vector4u;          \
  using prefix##Vector2f    = typename prefix##CoreAliases::Vector2f;          \
  using prefix##Vector3f    = typename prefix##CoreAliases::Vector3f;          \
  using prefix##Vector4f    = typename prefix##CoreAliases::Vector4f;          \
  using prefix##Vector2d    = typename prefix##CoreAliases::Vector2d;          \
  using prefix##Vector3d    = typename prefix##CoreAliases::Vector3d;          \
  using prefix##Vector4d    = typename prefix##CoreAliases::Vector4d;          \
  using prefix##Normal3f    = typename prefix##CoreAliases::Normal3f;          \
  using prefix##Normal3d    = typename prefix##CoreAliases::Normal3d;          \
  using prefix##Point2i     = typename prefix##CoreAliases::Point2i;           \
  using prefix##Point3i     = typename prefix##CoreAliases::Point3i;           \
  using prefix##Point4i     = typename prefix##CoreAliases::Point4i;           \
  using prefix##Point2u     = typename prefix##CoreAliases::Point2u;           \
  using prefix##Point3u     = typename prefix##CoreAliases::Point3u;           \
  using prefix##Point4u     = typename prefix##CoreAliases::Point4u;           \
  using prefix##Point2f     = typename prefix##CoreAliases::Point2f;           \
  using prefix##Point3f     = typename prefix##CoreAliases::Point3f;           \
  using prefix##Point4f     = typename prefix##CoreAliases::Point4f;           \
  using prefix##Point2d     = typename prefix##CoreAliases::Point2d;           \
  using prefix##Point3d     = typename prefix##CoreAliases::Point3d;           \
  using prefix##Point4d     = typename prefix##CoreAliases::Point4d;           \
  using prefix##Matrix2f    = typename prefix##CoreAliases::Matrix2f;          \
  using prefix##Matrix2d    = typename prefix##CoreAliases::Matrix2d;          \
  using prefix##Matrix3f    = typename prefix##CoreAliases::Matrix3f;          \
  using prefix##Matrix3d    = typename prefix##CoreAliases::Matrix3d;          \
  using prefix##Matrix4f    = typename prefix##CoreAliases::Matrix4f;          \
  using prefix##Matrix4d    = typename prefix##CoreAliases::Matrix4d;          \
  using prefix##Quat4f      = typename prefix##CoreAliases::Quat4f;            \
  using prefix##Quat4d      = typename prefix##CoreAliases::Quat4d;            \
  using prefix##Color1f     = typename prefix##CoreAliases::Color1f;           \
  using prefix##Color3f     = typename prefix##CoreAliases::Color3f;           \
  using prefix##Color4f     = typename prefix##CoreAliases::Color4f;           \
  using prefix##Color1d     = typename prefix##CoreAliases::Color1d;           \
  using prefix##Color3d     = typename prefix##CoreAliases::Color3d;           \
  using prefix##Color4d     = typename prefix##CoreAliases::Color4d;           \
  using prefix##Transform3f = typename prefix##CoreAliases::Transform3f;       \
  using prefix##Transform4f = typename prefix##CoreAliases::Transform4f;       \
  using prefix##Transform3d = typename prefix##CoreAliases::Transform3d;       \
  using prefix##Transform4d = typename prefix##CoreAliases::Transform4d;

// Variadic macro to import a set of variables from the base class
#define __EL_USING_TYPES_MACRO__(x) using typename Base::x;
#define EL_USING_TYPES(...) EL_MAP(__EL_USING_TYPES_MACRO__, __VA_ARGS__)

// Variadic macro to import a set of variables from the base class
#define __EL_USING_MEMBERS_MACRO__(x) using Base::x;
#define EL_USING_MEMBERS(...) EL_MAP(__EL_USING_MEMBERS_MACRO__, __VA_ARGS__)

#define EL_IMPORT_CORE_TYPES() EL_IMPORT_CORE_TYPES_PREFIX(Float, )

#define EL_IMPORT_CORE_TYPES_SCALAR() EL_IMPORT_CORE_TYPES_PREFIX(float, )
