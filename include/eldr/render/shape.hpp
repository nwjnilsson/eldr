#pragma once

#include <eldr/core/fwd.hpp>
#include <eldr/vulkan/fwd.hpp>

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

enum class ShapeType { Mesh, Disk, Rectangle, Sphere, Other };

class Shape {
  ELDR_IMPORT_CORE_TYPES();

public:
  // #ifdef ELDR_ENABLE_EMBREE
  //   virtual RTCGeometry embreeGeometry(RTCDevice device);
  // #endif

  virtual ~Shape() = default;

protected:
  inline Shape() {};

protected:
  // BSDF bsdf_;
  //  Emitter emitter_;
  //  Sensor sensor_;
  //  Medium interior_medium_;
  //  Medium exterior_medium_;
  // std::string id_;
  ShapeType shape_type_ = ShapeType::Other;
};
} // namespace eldr
