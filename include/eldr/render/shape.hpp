#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

#include <string>
namespace eldr {

// struct RenderObject {
//   ELDR_IMPORT_CORE_TYPES();
//   uint32_t index_count;
//   uint32_t first_index;
//   VkBuffer index_buffer; // this will be taken from render graph probably
//
//   // MaterialInstance* material;
//   Mat4f           transform;
//   VkDeviceAddress vertex_buffer_address;
// };
//
// struct DrawContext {
//   std::vector<RenderObject> opaque_surfaces;
// };

enum class ShapeType : uint8_t { Mesh, Disk, Rectangle, Sphere, Other };

class Shape {
  ELDR_IMPORT_CORE_TYPES();

public:
  // #ifdef ELDR_ENABLE_EMBREE
  //   virtual RTCGeometry embreeGeometry(RTCDevice device);
  // #endif
  virtual ~Shape() = default;

  /// @brief Get the name of this shape
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] ShapeType          type() const { return shape_type_; }

protected:
  // inline Shape() = default;
  inline Shape(const std::string& name, ShapeType type)
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
} // namespace eldr
