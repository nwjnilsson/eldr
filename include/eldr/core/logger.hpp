#pragma once

#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>

// TODO: define exceptions in a header and throw more specific things, like
// VkException for example

#define Throw(...)                                                             \
  do {                                                                         \
    spdlog::critical(__VA_ARGS__);                                             \
    throw std::runtime_error(                                                  \
      fmt::format("File: {}\n, line: {}\n {}",                                 \
                  std::filesystem::path(__FILE__).filename().string(),         \
                  __LINE__, fmt::format(__VA_ARGS__)));                        \
  } while (0)

#define ThrowVk(...) Throw("[VULKAN]: {}", __VA_ARGS__);
