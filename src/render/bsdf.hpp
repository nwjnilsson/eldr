#pragma once
#include <core/fwd.hpp>
#include <eldr.hpp>
#include <render/interaction.hpp>

NAMESPACE_BEGIN(eldr)

EL_VARIANT class BSDF {
public:
  virtual Spectrum eval() const = 0;
  virtual ~BSDF();
};
NAMESPACE_END(eldr)
