#pragma once
#include <eldr/core/fwd.hpp>

#include <string>

NAMESPACE_BEGIN(eldr::core)
class Formatter {
  friend Logger;

protected:
  Formatter() = default;

public:
  virtual ~Formatter() = default;

  [[nodiscard]] virtual std::string format(const Thread*      thread,
                                           const std::string& class_,
                                           const char*        function,
                                           const char*        file,
                                           int                line,
                                           const std::string& message) = 0;
};

class DefaultFormatter : public Formatter {
public:
  enum class ClassFuncFormat {
    None,         // Include neither
    ClassAndFunc, // Include class name (if available) and short function
                  // signature
    ClassOrFunc,  // Include class, fall back to short function
                  // signature if no class is available
    ClassOnly,    // Include class name, may be Unknown/anonymous
    FuncOnly,     // Include full function signature (gcc and msvc)
  };

public:
  DefaultFormatter() : Formatter() {};

  [[nodiscard]] std::string format(const Thread*      thread,
                                   const std::string& class_,
                                   const char*        function,
                                   const char*        file,
                                   int                line,
                                   const std::string& message) override;

  void setHasDate(bool has_date) { has_date_ = has_date; }
  void setHasTime(bool has_time) { has_time_ = has_time; }
  void setHasLogLevel(bool has_log_level) { has_log_level_ = has_log_level; }
  void setHasFile(bool has_file) { has_file_ = has_file; }
  void setHasThread(bool has_thread) { has_thread_ = has_thread; }
  void setClassFuncFormat(ClassFuncFormat format)
  {
    class_func_format_ = format;
  }

protected:
  bool has_date_{ true };
  bool has_time_{ true };
  bool has_log_level_{ true };
  bool has_thread_{ true };
  bool has_file_{ false };

  ClassFuncFormat class_func_format_{ ClassFuncFormat::ClassOrFunc };
};
NAMESPACE_END(eldr::core)
