#pragma once

#include <eldr/vulkan/fwd.hpp>
#include <vk_mem_alloc.h> // includes vulkan.h
#include <eldr/vulkan/exception.hpp>

namespace eldr::vk {
constexpr uint8_t max_frames_in_flight = 2;

namespace wr {
constexpr uint32_t required_vk_api_version = VK_API_VERSION_1_3;
}
} // namespace eldr::vk
