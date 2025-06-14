#pragma once
#include <eldr/core/logger.hpp>

#include <filesystem>
#include <fstream>
#include <mutex>

NAMESPACE_BEGIN(eldr)
struct ThreadingPolicy {};

struct SingleThreaded : ThreadingPolicy {
  void acquireLock() {}
};

struct MultiThreaded : ThreadingPolicy {
  std::mutex                   mutex;
  std::unique_lock<std::mutex> acquireLock() { return std::unique_lock(mutex); }
};

class Sink {
protected:
  Sink() = default;

public:
  virtual ~Sink() = default;

  virtual void operator()(LogLevel level, const std::string& text) = 0;

  /// @brief Process a progress message
  /// @param formatted Formatted string representation of the message
  virtual void logProgress(const std::string& formatted) = 0;

  template <typename Derived> [[nodiscard]] Derived*       as();
  template <typename Derived> [[nodiscard]] const Derived* as() const;
};

template <typename TPolicy> class StreamSink : public Sink {
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
  TPolicy       threading_;
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
  std::ofstream* openFile(const std::filesystem::path& file_path);

private:
  std::filesystem::path          file_path_;
  std::unique_ptr<std::ofstream> ofstream_;
};

//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Sink
// -----------------------------------------------------------------------------
template <typename Derived> [[nodiscard]] Derived* Sink::as()
{
  return dynamic_cast<Derived*>(this);
}

template <typename Derived> [[nodiscard]] const Derived* Sink::as() const
{
  return dynamic_cast<const Derived*>(this);
}

// -----------------------------------------------------------------------------
// StreamSink
// -----------------------------------------------------------------------------
template <typename T> struct isValidThreadingPolicy : std::false_type {};
template <> struct isValidThreadingPolicy<SingleThreaded> : std::true_type {};
template <> struct isValidThreadingPolicy<MultiThreaded> : std::true_type {};

template <typename TPolicy>
StreamSink<TPolicy>::StreamSink(std::ostream* stream) : Sink(), stream_(stream)
{
  static_assert(isValidThreadingPolicy<TPolicy>::value,
                "Invalid threading policy type");
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
    // Paint output
    // Insert a newline if the last message was a progress message
    if (last_message_was_progress_)
      (*stream_) << "\n";

    // The position of %L (if present) is the same on every line. Find the first
    // occurrence.
    size_t level_pos{ text.find("%L") };
    if (unlikely(level_pos == std::string::npos)) {
      level_pos = 0;
    }

    std::istringstream iss{ text };
    std::string        text_line;
    while (std::getline(iss, text_line)) {
      //  Append first part up to log level
      (*stream_) << text_line.substr(0, level_pos);
#if defined(_WIN32)
      HANDLE                     console = nullptr;
      CONSOLE_SCREEN_BUFFER_INFO console_info;
      memset(&console_info, 0, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
      console = GetStdHandle(STD_OUTPUT_HANDLE);
      GetConsoleScreenBufferInfo(console, &console_info);
      switch (level) {
        case Trace:
          SetConsoleTextAttribute(console, FOREGROUND_INTENSITY);
          (*stream_) << "TRACE";
          break;
        case Debug:
          SetConsoleTextAttribute(console, GREEN);
          (*stream_) << "DEBUG";
          break;
        case Info:
          SetConsoleTextAttribute(console, BLUE);
          (*stream_) << "INFO";
          break;
        case Warn:
          SetConsoleTextAttribute(console, YELLOW);
          (*stream_) << "WARNING";
          break;
        case Error:
          SetConsoleTextAttribute(console, DARKRED);
          (*stream_) << "ERROR";
          break;
        case Critical:
          SetConsoleTextAttribute(console, RED);
          (*stream_) << "CRITICAL";
          break;
        default:
          break;
      }
#else
      switch (level) {
        case Trace:
          (*stream_) << "\x1b[38;5;245m";
          (*stream_) << "TRACE";
          break;
        case Debug:
          (*stream_) << "\x1b[1;32m";
          (*stream_) << "DEBUG";
          break;
        case Info:
          (*stream_) << "\x1b[1;34m";
          (*stream_) << "INFO";
          break;
        case Warn:
          (*stream_) << "\x1b[1;33m";
          (*stream_) << "WARNING";
          break;
        case Error:
          (*stream_) << "\x1b[0;31m";
          (*stream_) << "ERROR";
          break;
        case Critical:
          (*stream_) << "\x1b[1;31m";
          (*stream_) << "CRITICAL";
          break;
        default:
          break;
      }
#endif
        // Reset text color
#if defined(_WIN32)
      SetConsoleTextAttribute(console, console_info.wAttributes);
#else
      (*stream_) << "\x1b[0m";
#endif

      (*stream_) << text_line.substr(level_pos + 2, text.length() - 1) << "\n";
    }
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
  if (ofstream_->fail()) {
    Log(Error, "Failed to write to file!");
  }
}

template <typename TPolicy>
std::ofstream*
FileStreamSink<TPolicy>::openFile(const std::filesystem::path& file_path)
{
  // fstream closes file automatically when destroyed
  ofstream_ = std::make_unique<std::ofstream>(
    file_path, std::fstream::in | std::fstream::out | std::fstream::trunc);

  if (ofstream_->fail()) {
    Log(Error, "Failed to open target sink file");
  }
  return ofstream_.get();
}
NAMESPACE_END(eldr)
