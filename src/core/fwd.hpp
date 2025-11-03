#pragma once
#include "embr/fwd.hpp"
#include "embr/traits.hpp"

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

namespace em = embr;

template <typename Value, std::size_t Size> struct Vector;
template <typename Value, std::size_t Size> struct Normal;
template <typename Value, std::size_t Size> struct Point;
template <typename Point> struct Transform;
template <typename Value, size_t Size> struct Spectrum;
template <typename Value, size_t Channels>
  requires(Channels <= 4)
struct Color;

template <typename Point, typename Spectrum> struct Ray;

template <typename _Float> struct CoreAliases {
  using Float = _Float;

  using Int8    = em::int8_array_t<Float>;
  using Int32   = em::int32_array_t<Float>;
  using UInt32  = em::uint32_array_t<Float>;
  using Int64   = em::int64_array_t<Float>;
  using UInt64  = em::uint64_array_t<Float>;
  using Float32 = em::float32_array_t<Float>;
  using Float64 = em::float64_array_t<Float>;

  using Vector2i = Vector<Int32, 2>;
  using Vector3i = Vector<Int32, 3>;
  using Vector4i = Vector<Int32, 4>;

  using Vector2u = Vector<UInt32, 2>;
  using Vector3u = Vector<UInt32, 3>;
  using Vector4u = Vector<UInt32, 4>;

  using Vector2f = Vector<Float, 2>;
  using Vector3f = Vector<Float, 3>;
  using Vector4f = Vector<Float, 4>;

  using Vector2d = Vector<Float64, 2>;
  using Vector3d = Vector<Float64, 3>;
  using Vector4d = Vector<Float64, 4>;

  using Normal3f = Vector<Float, 3>;
  using Normal3d = Vector<Float64, 3>;

  using Point2i = Vector<Int32, 2>;
  using Point3i = Vector<Int32, 3>;
  using Point4i = Vector<Int32, 4>;

  using Point2u = Vector<UInt32, 2>;
  using Point3u = Vector<UInt32, 3>;
  using Point4u = Vector<UInt32, 4>;

  using Point2f = Vector<Float, 2>;
  using Point3f = Vector<Float, 3>;
  using Point4f = Vector<Float, 4>;

  using Point2d = Vector<Float64, 2>;
  using Point3d = Vector<Float64, 3>;
  using Point4d = Vector<Float64, 4>;

  using Matrix2f = em::Matrix<Float, 2>;
  using Matrix3f = em::Matrix<Float, 3>;
  using Matrix4f = em::Matrix<Float, 4>;

  using Matrix2d = em::Matrix<Float64, 2>;
  using Matrix3d = em::Matrix<Float64, 3>;
  using Matrix4d = em::Matrix<Float64, 4>;

  using Quat4f = em::Quaternion<Float>;
  using Quat4d = em::Quaternion<Float64>;

  using Color1f = Color<Float, 1>;
  using Color3f = Color<Float, 3>;
  using Color4f = Color<Float, 4>;

  using Color1d = Color<Float64, 1>;
  using Color3d = Color<Float64, 3>;
  using Color4d = Color<Float64, 4>;

  using Transform3f = Transform<Point<Float, 3>>;
  using Transform4f = Transform<Point<Float, 4>>;
  using Transform3d = Transform<Point<Float64, 3>>;
  using Transform4d = Transform<Point<Float64, 4>>;
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

// using ScalarFloat = eldr::math::scalar_t<Float>;
#define EL_IMPORT_CORE_TYPES() EL_IMPORT_CORE_TYPES_PREFIX(Float, )
