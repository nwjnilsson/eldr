#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <string>
NAMESPACE_BEGIN(eldr::render)

enum class ShapeType : uint8_t { Mesh, Disk, Rectangle, Sphere, Other };

EL_VARIANT class Shape {
  EL_IMPORT_CORE_TYPES()

public:
  // #ifdef EL_ENABLE_EMBREE
  //   virtual RTCGeometry embreeGeometry(RTCDevice device);
  // #endif
  virtual ~Shape() = default;

  /// @brief Get the name of this shape
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] ShapeType          type() const { return shape_type_; }

protected:
  // inline Shape() = default;
  inline Shape(std::string_view name, ShapeType type)
    : name_(name), shape_type_(type) {};

protected:
  std::string name_;
  // BSDF bsdf_;
  //  Emitter emitter_;
  //  Sensor sensor_;
  //  Medium interior_medium_;
  //  Medium exterior_medium_;
  ShapeType shape_type_{ ShapeType::Other };
};
NAMESPACE_END(eldr::render)
