#include <core/core.hpp>
#include <eldr.hpp>
#include <render/endpoint.hpp>

NAMESPACE_BEGIN(eldr)

EL_VARIANT Endpoint<Float, Spectrum>::Endpoint()  = default;
EL_VARIANT Endpoint<Float, Spectrum>::~Endpoint() = default;
EL_INSTANTIATE_CLASS(Endpoint)

NAMESPACE_END(eldr)
