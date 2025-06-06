#include <eldr/core/logger.hpp>
#include <eldr/core/progress.hpp>
#include <eldr/core/thread.hpp>
#include <eldr/core/util.hpp>

#include <chrono>

namespace eldr::core {
ProgressReporter::ProgressReporter(std::string_view label, void* payload)
  : label_(label), line_(util::terminalWidth() + 1, ' '),
    bar_start_(label.length() + 3), payload_(payload)
{
  line_[0] = '\r';

  // Format of progress bar is:
  // without ETA:
  // Name: [==========>   ] XX%
  // with ETA:
  // Name: [==========>   ] XX% | ETA: XXh YYmin ZZs
  // Width of entire progress bar is 'columns' characters
  // Need to calculate number of equal signs that go into the middle
  // ':' + <space> + '[' + '>' + ']' + <space> + "XX%" = 9 characters
  const size_t bar_size{
    line_.length() - bar_start_ - 2 //  trailing bracket and space
    - eta_max_length                // max eta string length};
  };
  if (bar_size > 0) {
    bar_size_ = bar_size;
    memcpy(static_cast<char*>(line_.data()) + 1, label.data(), label.length());
    line_[bar_start_ - 1]         = '[';
    line_[bar_start_ + bar_size_] = ']';
  }
}

void ProgressReporter::update(float progress)
{
  if (progress < 0.f or progress > 1.f) {
    Log(Debug, "'progress' is outside the range [0, 100]!");
    progress = std::min(1.f, std::max(0.f, progress));
  }

  if (progress == last_progress_) {
    return;
  }

  const long elapsed{ timer_.millis(false) };
  if (progress != 1.f and (elapsed - last_update_ < 500 or
                           std::abs(progress - last_progress_) < 0.01f)) {
    return;
  }

  const size_t       remaining{ static_cast<size_t>(elapsed / progress *
                                              (1.f - progress)) };
  std::ostringstream oss;
  using ms = std::chrono::milliseconds;
  const std::chrono::hh_mm_ss<ms> eta_time{ ms(remaining) };
  oss << " | " << eta_time.hours() << " " << eta_time.minutes() << " "
      << eta_time.seconds();

  std::string eta{ oss.str() };
  if (eta.length() > eta_max_length) {
    eta.resize(eta_max_length);
    return;
  }

  if (bar_size_ > 0) {
    const size_t progress_count{ std::min(
      bar_size_, static_cast<size_t>(std::round(bar_size_ * progress))) };
    size_t       eta_pos{ bar_start_ + bar_size_ + 2 };
    size_t       arrowhead_pos{ bar_start_ + progress_count - 1 };
    // clang-format off
    memset(static_cast<char*>(line_.data()) + bar_start_, '=', progress_count - 1);
    memset(static_cast<char*>(line_.data()) + arrowhead_pos, '>', 1);
    memset(static_cast<char*>(line_.data()) + eta_pos, ' ', line_.length() - eta_pos - 1);
    memcpy(static_cast<char*>(line_.data()) + eta_pos, eta.data(), eta.length());
    // clang-format on
  }

  Thread::thread()->logger()->logProgress(line_);
  last_update_ = elapsed;
}
} // namespace eldr::core
