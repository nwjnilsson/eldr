#pragma once

#include <eldr/vulkan/common.hpp>

#include <spdlog/fmt/ostr.h>

extern std::ostream& operator<<(std::ostream& os, const VkResult&);

template <>
struct fmt::formatter<VkResult> : fmt::ostream_formatter {};
