#pragma once
#include <eldr/core/platform.hpp>
#include <eldr/math/fwd.hpp>

#include <glm/fwd.hpp>

#define EL_VARIANT template <typename Float, typename Spectrum>

using FlagRep = uint32_t;
using byte_t  = std::byte;

namespace eldr {
namespace core {
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

template <typename T, size_t Size> struct Color;
template <typename Value, size_t Size> struct Spectrum;
template <typename Point, typename Spectrum> struct Ray;

} // namespace core

// TODO: experiment with glm SIMD types and compare performance
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

  using Vector2i = em::Vector<Int32, 2>;
  using Vector3i = em::Vector<Int32, 3>;
  using Vector4i = em::Vector<Int32, 4>;

  using Vector2u = em::Vector<UInt32, 2>;
  using Vector3u = em::Vector<UInt32, 3>;
  using Vector4u = em::Vector<UInt32, 4>;

  using Vector2f = em::Vector<Float, 2>;
  using Vector3f = em::Vector<Float, 3>;
  using Vector4f = em::Vector<Float, 4>;

  using Vector2d = em::Vector<Float64, 2>;
  using Vector3d = em::Vector<Float64, 3>;
  using Vector4d = em::Vector<Float64, 4>;

  using Normal3f = em::Normal<Float, 3>;
  using Normal3d = em::Normal<Float64, 3>;

  using Point2i = em::Point<Int32, 2>;
  using Point3i = em::Point<Int32, 3>;
  using Point4i = em::Point<Int32, 4>;

  using Point2u = em::Point<UInt32, 2>;
  using Point3u = em::Point<UInt32, 3>;
  using Point4u = em::Point<UInt32, 4>;

  using Point2f = em::Point<Float, 2>;
  using Point3f = em::Point<Float, 3>;
  using Point4f = em::Point<Float, 4>;

  using Point2d = em::Point<Float64, 2>;
  using Point3d = em::Point<Float64, 3>;
  using Point4d = em::Point<Float64, 4>;

  using Matrix2f = em::Matrix<Float, 2>;
  using Matrix3f = em::Matrix<Float, 3>;
  using Matrix4f = em::Matrix<Float, 4>;

  using Matrix2d = em::Matrix<Float64, 2>;
  using Matrix3d = em::Matrix<Float64, 3>;
  using Matrix4d = em::Matrix<Float64, 4>;

  using Quat4f = glm::qua<Float>; // TODO: wrap
  using Quat4d = glm::qua<Float64>;

  using Color1f = core::Color<Float, 1>;
  using Color3f = core::Color<Float, 3>;
  using Color4f = core::Color<Float, 4>;

  using Color1d = core::Color<Float64, 1>;
  using Color3d = core::Color<Float64, 3>;
  using Color4d = core::Color<Float64, 4>;

  using Transform3f = em::Transform<Point3f>;
  using Transform4f = em::Transform<Point4f>;
  using Transform3d = em::Transform<Point3d>;
  using Transform4d = em::Transform<Point4d>;
};
} // namespace eldr

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
