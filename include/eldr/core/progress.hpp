#pragma once
#include <eldr/core/stopwatch.hpp>
NAMESPACE_BEGIN(eldr::core)
class ProgressReporter {
public:
  ProgressReporter(std::string_view label, void* payload = nullptr);
  ~ProgressReporter() = default;

  void update(float progress);

private:
  static constexpr size_t eta_max_length{ 25 };

protected:
  StopWatch   timer_;
  std::string label_;
  std::string line_;
  size_t      bar_start_;
  size_t      bar_size_{ 0 };
  size_t      last_update_{ 0 };
  float       last_progress_{ -1.f };
  void*       payload_;
};
NAMESPACE_END(eldr::core)
