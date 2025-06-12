#pragma once
#include <eldr/vulkan/fwd.hpp>
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
#  include <vk_mem_alloc.h> // includes vulkan.h
#  pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
// TODO: try compiling on windows and disable warnings corresponding to the ones
// above
// #  pragma warning(disable:xxxx)
#  include <vk_mem_alloc.h> // includes vulkan.h
// #  pragma warning(default:xxxx)
#endif
// =============================================================================
#include <eldr/core/logger.hpp>
#include <eldr/vulkan/vktools/format.hpp>
NAMESPACE_BEGIN(eldr::vk)
constexpr uint32_t max_frames_in_flight{ 2 };
constexpr uint32_t required_vk_api_version{ VK_API_VERSION_1_3 };
NAMESPACE_END(eldr::vk)
