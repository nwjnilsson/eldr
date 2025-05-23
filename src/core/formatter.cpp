#include <eldr/core/formatter.hpp>
#include <eldr/core/logger.hpp>

#include <fmt/args.h>
#include <fmt/format.h>

#include <chrono>
#include <filesystem>
#include <iostream>

namespace eldr::core {

std::string DefaultFormatter::format(const Thread*      thread,
                                     const std::string& class_,
                                     const char*        function,
                                     const char*        file,
                                     int                line,
                                     const std::string& message)
{
  using namespace std::chrono;
  // TODO: std::chrono in C++20 can do all this in a cleaner way but I need
  // GCC13, which I don't have atm
  char   buffer[128];
  time_t time{ std::time(nullptr) };
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.", std::localtime(&time));
  time_point now{ system_clock::now() };
  hh_mm_ss   time_of_day{ now - floor<days>(now) };
  long millis = duration_cast<milliseconds>(time_of_day.subseconds()).count();

  std::ostringstream oss;
  oss << buffer;
  if (unlikely(millis < 100)) {
    oss << "0";
  }
  oss << millis;
  const std::string time_full{ oss.str() };
  oss.str("");
  oss.clear();
  std::istringstream iss{ message };
  std::string        msg_line;
  int                line_idx{ 0 };
  while (std::getline(iss, msg_line) || line_idx == 0) {
    if (line_idx > 0) {
      oss << '\n';
    }

    if (has_date_) {
      oss << "[" << time_full << "] ";
    }

    if (has_log_level_) {
      oss << "[%L] "; // replaced in sink
                      // TODO: pad like thread names below using log level?
    }

    if (thread and has_thread_) {
      oss << thread->name();
      for (int i{ 0 }; i < (6 - static_cast<int>(thread->name().length())); ++i)
        oss << ' ';
    }

    const std::string class_name{ class_ };
    switch (class_func_format_) {
      case ClassFuncFormat::None:
        break;
      case ClassFuncFormat::ClassAndFunc:
        if (class_name.compare("Unknown") != 0) {
          oss << "[" << class_name << "] ";
        }
        oss << "[" << function << "()] ";
        break;
      case ClassFuncFormat::ClassOrFunc:
        if (class_name.compare("Unknown") != 0) {
          oss << "[" << class_name << "] ";
        }
        else {
          oss << "[" << function << "()] ";
        }
        break;
      case ClassFuncFormat::ClassOnly:
        oss << "[" << class_name << "] ";
        break;
      case ClassFuncFormat::FuncOnly:
        oss << "[" << function << "()] ";
        break;
    }

    if (has_file_) {
      oss << "[" << std::filesystem::path(file).filename() << ":" << line
          << "]: ";
    }

    oss << msg_line;
    line_idx++;
  }
  return oss.str();
}
} // namespace eldr::core
