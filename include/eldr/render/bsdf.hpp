#pragma once
#include <eldr/core/fwd.hpp>
#include <eldr/eldr.hpp>
#include <eldr/render/interaction.hpp>

NAMESPACE_BEGIN(eldr)

EL_VARIANT class BSDF {
public:
  virtual Spectrum eval() const = 0;
  virtual ~BSDF();
};
NAMESPACE_END(eldr)
