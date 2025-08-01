#pragma once
#include <eldr/render/endpoint.hpp>
NAMESPACE_BEGIN(eldr)
class Sensor : public Endpoint {
public:
  ~Sensor() = default;

protected:
  Sensor() = default;

private:
  // film
  // resolution
  // shutter time
};

NAMESPACE_END(eldr)
