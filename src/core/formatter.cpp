#include <eldr/core/formatter.hpp>
#include <eldr/core/logger.hpp>

#include <fmt/args.h>
#include <fmt/format.h>

#include <chrono>
#include <filesystem>

namespace eldr::core {

std::string DefaultFormatter::format(LogLevel           level,
                                     const Thread*      thread,
                                     const char*        class_,
                                     const char*        function,
                                     const char*        file,
                                     int                line,
                                     const std::string& message)
{
  using namespace std::chrono;
  // std::chrono in C++20 can do all this in a cleaner way but I need GCC13,
  // which I don't have atm
  char   buffer[128];
  time_t time{ std::time(nullptr) };
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.", std::localtime(&time));
  time_point now{ system_clock::now() };
  hh_mm_ss   time_of_day{ now - floor<days>(now) };
  auto       millis = duration_cast<milliseconds>(time_of_day.subseconds());

  std::ostringstream oss;
  oss << buffer << millis.count() << " ";
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
      oss << time_full;
    }

    switch (level) {
      case Trace:
        oss << "[TRACE] ";
        break;
      case Debug:
        oss << "[DEBUG] ";
        break;
      case Info:
        oss << "[INFO] ";
        break;
      case Warn:
        oss << "[WARNING] ";
        break;
      case Error:
        oss << "[ERROR] ";
        break;
      case Critical:
        oss << "[CRITICAL] ";
        break;
      default:
        oss << "[UNKNOWN] ";
        break;
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
        if (not class_name.compare("Unknown")) {
          oss << "[" << class_name << "] ";
        }
        oss << "[" << function << "()] ";
        break;
      case ClassFuncFormat::ClassOrFunc:
        if (not class_name.compare("Unknown")) {
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
