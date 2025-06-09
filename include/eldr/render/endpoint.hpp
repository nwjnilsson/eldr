#pragma once
#include <eldr/render/fwd.hpp>
#include <string>
namespace eldr::render {
EL_VARIANT class Endpoint {
  EL_IMPORT_TYPES(Medium, Shape);

public:
  ~Endpoint();

protected:
  Endpoint();

private:
  // std::unique_ptr<Transform4f> to_world_;
  // std::unique_ptr<Medium>      medium_;
  Shape*      shape_{ nullptr };
  std::string id_;
};
} // namespace eldr::render
