#pragma once
#include <eldr/vulkan/fwd.hpp>

#include <spdlog/fmt/ostr.h>
#include <vulkan/vulkan.h>

namespace std {
// Overloaded operator<< need to be in std namespace for ADL
std::ostream& operator<<(std::ostream& os, const VkResult&);
} // namespace std

template <> struct fmt::formatter<VkResult> : fmt::ostream_formatter {};
