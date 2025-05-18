#pragma once
#include <eldr/core/logger.hpp>

#include <filesystem>
#include <fstream>
#include <mutex>

namespace eldr::core {

struct ThreadingPolicy {};
struct SingleThreaded : ThreadingPolicy {
  void acquireLock() {}
};
struct MultiThreaded : ThreadingPolicy {
  std::mutex                   mutex;
  std::unique_lock<std::mutex> acquireLock() { return std::unique_lock(mutex); }
};

template <typename TPolicy> class Sink {
public:
protected:
  Sink();

public:
  virtual ~Sink() = default;

  virtual void operator()(LogLevel level, const std::string& text) = 0;

  /// @brief Process a progress message
  /// @param formatted Formatted string representation of the message
  virtual void logProgress(const std::string& formatted) = 0;

  template <typename Derived> [[nodiscard]] Derived*       as();
  template <typename Derived> [[nodiscard]] const Derived* as() const;

protected:
  TPolicy threading_;
};

template <typename TPolicy> class StreamSink : public Sink<TPolicy> {
public:
  StreamSink(std::ostream* stream);

  virtual ~StreamSink() = default;

  /// Calls this sink object, appending `text` to the output stream
  virtual void operator()(LogLevel level, const std::string& text) override;

  /// Log progress
  virtual void logProgress(const std::string& formatted) override;

protected:
  void sink(LogLevel level, const std::string& text, bool color_output);

protected:
  std::ostream* stream_;
  bool          last_message_was_progress_{ false };
};

template <typename TPolicy>
class FileStreamSink final : public StreamSink<TPolicy> {
public:
  FileStreamSink(const std::filesystem::path& file_path);

public:
  void operator()(LogLevel level, const std::string& text) override;

private:
  std::ostream* openFile(const std::filesystem::path& file_path)
  {
    // fstream closes file automatically when destroyed
    fstream_ = std::make_unique<std::fstream>(
      file_path, std::fstream::in | std::fstream::out | std::fstream::trunc);

    if (fstream_->fail()) {
      Log(Error, "Failed to open target sink file");
    }
    return fstream_.get();
  }

private:
  std::filesystem::path         file_path_;
  std::unique_ptr<std::ostream> fstream_;
};

//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Sink
// -----------------------------------------------------------------------------
template <typename T> struct isValidThreadingPolicy : std::false_type {};
template <> struct isValidThreadingPolicy<SingleThreaded> : std::true_type {};
template <> struct isValidThreadingPolicy<MultiThreaded> : std::true_type {};

template <typename TPolicy> Sink<TPolicy>::Sink()
{
  static_assert(isValidThreadingPolicy<TPolicy>::value,
                "Invalid threading policy type");
};

template <typename TPolicy>
template <typename Derived>
[[nodiscard]] Derived* Sink<TPolicy>::as()
{
  return dynamic_cast<Derived*>(this);
}

template <typename TPolicy>
template <typename Derived>
[[nodiscard]] const Derived* Sink<TPolicy>::as() const
{
  return dynamic_cast<const Derived*>(this);
}

// -----------------------------------------------------------------------------
// StreamSink
// -----------------------------------------------------------------------------
template <typename TPolicy>
StreamSink<TPolicy>::StreamSink(std::ostream* stream)
  : Sink<TPolicy>(), stream_(stream)
{
}

template <typename TPolicy>
void StreamSink<TPolicy>::operator()(LogLevel level, const std::string& text)
{
  sink(level, text, true);
}

template <typename TPolicy>
void StreamSink<TPolicy>::sink(LogLevel           level,
                               const std::string& text,
                               bool               color_output)
{
  this->threading_.acquireLock();

  if (color_output) {
    size_t level_pos{ 0 };
    size_t count{ 0 };
    auto   findPosAndCount = [&](std::string_view word) {
      level_pos = text.find(word);
      count     = word.length();
      if (unlikely(level_pos == std::string::npos)) {
        level_pos = 0;
        count     = 0;
      }
    };
    // Paint output
    // Insert a newline if the last message was a progress message
    if (last_message_was_progress_)
      (*stream_) << "\n";
    //  Append first part up to log level
    (*stream_) << text.substr(0, level_pos);
#if defined(_WIN32)
    HANDLE                     console = nullptr;
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    memset(&console_info, 0, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
    console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &console_info);
    switch (level) {
      case Trace:
        findPosAndCount("TRACE");
        SetConsoleTextAttribute(console, CYAN);
        break;
      case Debug:
        findPosAndCount("DEBUG");
        SetConsoleTextAttribute(console, BLUE);
        break;
      case Info:
        findPosAndCount("INFO");
        SetConsoleTextAttribute(console, FOREGROUND_INTENSITY);
        break;
      case Warn:
        findPosAndCount("WARNING");
        SetConsoleTextAttribute(console, YELLOW);
        break;
      case Error:
        findPosAndCount("ERROR");
        SetConsoleTextAttribute(console, RED);
        break;
      case Critical:
        findPosAndCount("CRITICAL");
        SetConsoleTextAttribute(console, DARKRED);
        break;
      default:
        break;
    }
#else
    switch (level) {
      case Trace:
        findPosAndCount("TRACE");
        (*stream_) << "\x1b[1;36m";
        break;
      case Debug:
        findPosAndCount("DEBUG");
        (*stream_) << "\x1b[1;34m";
        break;
      case Info:
        findPosAndCount("INFO");
        (*stream_) << "\x1b[38;5;245m";
        break;
      case Warn:
        findPosAndCount("WARNING");
        (*stream_) << "\x1b[1;33m";
        break;
      case Error:
        findPosAndCount("ERROR");
        (*stream_) << "\x1b[1;31m";
        break;
      case Critical:
        findPosAndCount("CRITICAL");
        (*stream_) << "\x1b[0;31m";
        break;
      default:
        break;
    }
#endif
    // Append log level
    (*stream_) << text.substr(level_pos, count);
    // Reset text color
#if defined(_WIN32)
    SetConsoleTextAttribute(console, console_info.wAttributes);
#else
    (*stream_) << "\x1b[0m";
#endif

    (*stream_) << text.substr(level_pos + count, text.length() - 1) << "\n";
  }
  else {
    // No painting, print as is
    (*stream_) << text << "\n";
  }
  last_message_was_progress_ = false;
}

template <typename TPolicy>
void StreamSink<TPolicy>::logProgress(const std::string& formatted)
{
  this->threading_.acquireLock();
  std::ostream& stream{ *stream_ };
  stream << formatted;
  stream.flush();
  last_message_was_progress_ = true;
}

// -----------------------------------------------------------------------------
// FileStreamSink
// -----------------------------------------------------------------------------
template <typename TPolicy>
FileStreamSink<TPolicy>::FileStreamSink(const std::filesystem::path& file_path)
  : StreamSink<TPolicy>(openFile(file_path)), file_path_(file_path)
{
  this->color_output = false;
}

template <typename TPolicy>
void FileStreamSink<TPolicy>::operator()(LogLevel           level,
                                         const std::string& text)
{
  StreamSink<TPolicy>::sink(level, text, false);
  if (fstream_->fail()) {
    Log(Error, "Failed to write to file!");
  }
}
} // namespace eldr::core
