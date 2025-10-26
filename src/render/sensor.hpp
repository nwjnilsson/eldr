#pragma once
#include <render/endpoint.hpp>
NAMESPACE_BEGIN(eldr)
EL_VARIANT class Sensor : public Endpoint<Float, Spectrum> {
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
