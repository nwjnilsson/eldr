#pragma once
#include <eldr/math/transform.hpp>
#include <eldr/render/fwd.hpp>
#include <memory>

namespace eldr::render {
EL_VARIANT class Endpoint {
  EL_IMPORT_TYPES(Medium, Shape);

public:
  ~Endpoint();

protected:
  Endpoint();

private:
  Transform4f             to_world_;
  std::unique_ptr<Medium> medium_;
  Shape*                  shape_{ nullptr };
  std::string             id_;
};
} // namespace eldr::render
