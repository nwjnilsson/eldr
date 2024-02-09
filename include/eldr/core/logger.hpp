#pragma once

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <filesystem>

// TODO: define exceptions in a header and throw more specific things, like
// VkException for example

#define Throw(...)                                                             \
  do {                                                                         \
    throw std::runtime_error(                                                  \
      fmt::format("[File: {}, line: {}] {}",                                   \
                  std::filesystem::path(__FILE__).filename().string(),         \
                  __LINE__, fmt::format(__VA_ARGS__)));                        \
  } while (0)

#define ThrowSpecific(except, ...)                                                 \
  do {                                                                         \
    throw except(                                                  \
      fmt::format("[File: {}, line: {}] {}",                                   \
                  std::filesystem::path(__FILE__).filename().string(),         \
                  __LINE__, fmt::format(__VA_ARGS__)));                        \
  } while (0)

#define ThrowVk(...) Throw("[VULKAN]: {}", __VA_ARGS__);
