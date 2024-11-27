#pragma once

#include <chrono>

namespace eldr::core {
class StopWatch {
public:
  StopWatch();
  ~StopWatch() = default;

  /// @brief Updates the current stopwatch timestamp and returns the time passed
  /// since the previous timestamp.
  /// @tparam Rep The type to represent the time in, usually float/int.
  /// @tparam Duration A std::chrono::duration representing the unit of time,
  /// normally std::chrono::seconds or std::chrono::milliseconds.
  /// @param reset Whether to update the latest timestamp or not. time(false)
  /// will return the time passed since the last updated timestamp, whether that
  /// is from a previous call to time(true) or since initialization.
  /// @return Rep The time, expressed as a Rep, since the previous timestamp.
  template <typename Rep, typename Duration> Rep time(bool reset = true)
  {
    const auto current{ std::chrono::high_resolution_clock::now() };
    Rep        time_since_last =
      std::chrono::duration<Rep, typename Duration::period>(current - latest_)
        .count();
    if (reset)
      latest_ = current;
    return time_since_last;
  }

  /// @brief Updates the current stopwatch timestamp and returns the time passed
  /// since the previous timestamp.
  /// @param reset Whether to update the latest timestamp or not. millis(false)
  /// will return the time passed since the last updated timestamp, whether that
  /// is from a previous call to millis(true) or since initialization.
  /// @return float The time, in milliseconds, since the previous timestamp.
  [[nodiscard]] float millis(bool reset = true)
  {
    return time<float, std::chrono::milliseconds>(reset);
  }
  /// @brief Updates the current stopwatch timestamp and returns the time passed
  /// since the previous timestamp.
  /// @param reset Whether to update the latest timestamp or not. seconds(false)
  /// will return the time passed since the last updated timestamp, whether that
  /// is from a previous call to seconds(true) or since initialization.
  /// @return float The time, in seconds, since the previous timestamp.
  [[nodiscard]] float seconds(bool reset = true)
  {
    return time<float, std::chrono::seconds>(reset);
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> init_;
  std::chrono::time_point<std::chrono::high_resolution_clock> latest_;
};
} // namespace eldr::core
