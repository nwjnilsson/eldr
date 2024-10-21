#pragma once

#include <eldr/vulkan/vktools/format.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
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
