#pragma once

#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>

// TODO: define exceptions in a header and throw more specific things, like
// VkException for example
#define Throw(...)                                                             \
  do {                                                                         \
    throw std::runtime_error(                                                  \
      fmt::format("[File: {}, line: {}] {}",                                   \
                  std::filesystem::path(__FILE__).filename().string(),         \
                  __LINE__, fmt::format(__VA_ARGS__)));                        \
  } while (0)

#define ThrowVk(result, ...)                                                   \
  do {                                                                         \
    throw eldr::vk::VulkanException(                                           \
      fmt::format("[File: {}, line: {}] {}",                                   \
                  std::filesystem::path(__FILE__).filename().string(),         \
                  __LINE__, fmt::format(__VA_ARGS__)),                         \
      result);                                                                 \
  } while (0)

namespace eldr::detail {
// TODO: not sure how to handle various loggers, but I guess this will work for
// now
std::shared_ptr<spdlog::logger> requestLogger(const std::string& name);
} // namespace eldr::detail
