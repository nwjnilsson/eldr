#pragma once
#include <eldr/core/exceptions.hpp>
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/fwd.hpp>
#include <spdlog/spdlog.h>
//------------------------------------------------------------------------------
// vk_mem_alloc.h gives rise to a lot of warnings about unused
// parameters/variables and missing field initializers, hence the suppression
//------------------------------------------------------------------------------
#ifdef __GNUG__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#ifdef _MSC_VER
// TODO: try compiling on windows and disable warnings corresponding to the ones
// above
// #  pragma warning(disable:xxxx)
#endif
#include <vk_mem_alloc.h> // includes vulkan.h
#ifdef __GNUG__
#  pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
// #  pragma warning(default:xxxx)
#endif
// =============================================================================
namespace eldr::vk {
constexpr uint32_t max_frames_in_flight    = 2;
constexpr uint32_t required_vk_api_version = VK_API_VERSION_1_3;
} // namespace eldr::vk
