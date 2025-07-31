#pragma once
#include <eldr/render/endpoint.hpp>
NAMESPACE_BEGIN(eldr)
class Sensor : public Endpoint {
public:
  ~Sensor();

protected:
  Sensor();

private:
  // film
  // resolution
  // shutter time
};

NAMESPACE_END(eldr)
