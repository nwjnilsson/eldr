#pragma once
#include <eldr/core/flags.hpp>
#include <eldr/core/platform.hpp>

#include <glm/fwd.hpp>
using Float = typename glm::float32_t;

using byte_t = std::byte;

namespace eldr {
class Bitmap;
class Struct;
class Stream;
class StopWatch;

namespace core {
enum LogLevel : int;
class Logger;
class Formatter;
struct ThreadingPolicy;
struct SingleThreaded;
struct MultiThreaded;
class Sink;
class Thread;
} // namespace core

template <size_t size, typename T> using Color = glm::vec<size, T>;
template <size_t size, typename T> using Point = glm::vec<size, T>;
template <size_t size, typename T> using Vec   = glm::vec<size, T>;

// TODO: experiment with glm SIMD types and compare performance
template <typename Float_> struct CoreAliases {
  using Float = Float_;

  using Int8    = glm::int8_t;
  using Int32   = glm::int32_t;
  using UInt32  = glm::uint32_t;
  using Int64   = glm::int64_t;
  using UInt64  = glm::uint64_t;
  using Float32 = glm::float32_t;
  using Float64 = glm::float64_t;

  using Vec2i = Vec<2, Int32>;
  using Vec3i = Vec<3, Int32>;
  using Vec4i = Vec<4, Int32>;

  using Vec2u = Vec<2, UInt32>;
  using Vec3u = Vec<3, UInt32>;
  using Vec4u = Vec<4, UInt32>;

  using Vec2f = Vec<2, Float>;
  using Vec3f = Vec<3, Float>;
  using Vec4f = Vec<4, Float>;

  using Vec2d = Vec<2, Float64>;
  using Vec3d = Vec<3, Float64>;
  using Vec4d = Vec<4, Float64>;

  using Point2i = Point<2, Int32>;
  using Point3i = Point<3, Int32>;
  using Point4i = Point<4, Int32>;

  using Point2u = Point<2, UInt32>;
  using Point3u = Point<3, UInt32>;
  using Point4u = Point<4, UInt32>;

  using Point2f = Point<2, Float>;
  using Point3f = Point<3, Float>;
  using Point4f = Point<4, Float>;

  using Point2d = Point<2, Float64>;
  using Point3d = Point<3, Float64>;
  using Point4d = Point<4, Float64>;

  using Mat2f = glm::mat<2, 2, Float>;
  using Mat3f = glm::mat<3, 3, Float>;
  using Mat4f = glm::mat<4, 4, Float>;

  using Mat2d = glm::mat<2, 2, Float64>;
  using Mat3d = glm::mat<3, 3, Float64>;
  using Mat4d = glm::mat<4, 4, Float64>;

  using Quat4f = glm::qua<Float>;
  using Quat4d = glm::qua<Float64>;

  using Color1f = Color<1, Float>;
  using Color3f = Color<3, Float>;
  using Color4f = Color<4, Float>;

  using Color1d = Color<1, Float64>;
  using Color3d = Color<3, Float64>;
  using Color4d = Color<4, Float64>;
};
} // namespace eldr

#define ELDR_IMPORT_CORE_TYPES_PREFIX(Float_, prefix)                          \
  using prefix##CoreAliases = eldr::CoreAliases<Float_>;                       \
  using prefix##Vec2i       = typename prefix##CoreAliases::Vec2i;             \
  using prefix##Vec3i       = typename prefix##CoreAliases::Vec3i;             \
  using prefix##Vec4i       = typename prefix##CoreAliases::Vec4i;             \
  using prefix##Vec2u       = typename prefix##CoreAliases::Vec2u;             \
  using prefix##Vec3u       = typename prefix##CoreAliases::Vec3u;             \
  using prefix##Vec4u       = typename prefix##CoreAliases::Vec4u;             \
  using prefix##Vec2f       = typename prefix##CoreAliases::Vec2f;             \
  using prefix##Vec3f       = typename prefix##CoreAliases::Vec3f;             \
  using prefix##Vec4f       = typename prefix##CoreAliases::Vec4f;             \
  using prefix##Vec2d       = typename prefix##CoreAliases::Vec2d;             \
  using prefix##Vec3d       = typename prefix##CoreAliases::Vec3d;             \
  using prefix##Vec4d       = typename prefix##CoreAliases::Vec4d;             \
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
  using prefix##Mat2f       = typename prefix##CoreAliases::Mat2f;             \
  using prefix##Mat2d       = typename prefix##CoreAliases::Mat2d;             \
  using prefix##Mat3f       = typename prefix##CoreAliases::Mat3f;             \
  using prefix##Mat3d       = typename prefix##CoreAliases::Mat3d;             \
  using prefix##Mat4f       = typename prefix##CoreAliases::Mat4f;             \
  using prefix##Mat4d       = typename prefix##CoreAliases::Mat4d;             \
  using prefix##Quat4f      = typename prefix##CoreAliases::Quat4f;            \
  using prefix##Quat4d      = typename prefix##CoreAliases::Quat4d;            \
  using prefix##Color1f     = typename prefix##CoreAliases::Color1f;           \
  using prefix##Color3f     = typename prefix##CoreAliases::Color3f;           \
  using prefix##Color4f     = typename prefix##CoreAliases::Color4f;           \
  using prefix##Color1d     = typename prefix##CoreAliases::Color1d;           \
  using prefix##Color3d     = typename prefix##CoreAliases::Color3d;           \
  using prefix##Color4d     = typename prefix##CoreAliases::Color4d;

#define ELDR_IMPORT_CORE_TYPES() ELDR_IMPORT_CORE_TYPES_PREFIX(Float, )
