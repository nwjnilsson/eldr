#pragma once
#include <eldr/core/fwd.hpp>

#include <memory>

namespace eldr::core {
class Formatter {
  friend Logger;

protected:
  Formatter();

public:
  virtual ~Formatter();

  [[nodiscard]] virtual std::string format(LogLevel           level,
                                           const Thread*      thread,
                                           const char*        function,
                                           const char*        file,
                                           int                line,
                                           const std::string& message) = 0;

protected:
};

class DefaultFormatter : public Formatter {
public:
  enum class ClassFuncFormat {
    None,            // Include neither
    ClassOrFunction, // Include class in formatted text, fall back to function
                     // signature if no class is available
    ClassOnly, // Include class name in formatted text, "Unknown" if no class is
               // available
    FunctionOnly, // Include function signature only
  };

public:
  DefaultFormatter() : Formatter() {};

  [[nodiscard]] std::string format(LogLevel           level,
                                   const Thread*      thread,
                                   const char*        function,
                                   const char*        file,
                                   int                line,
                                   const std::string& message) override;

  void setHasDate(bool has_date) { has_date_ = has_date; }
  void setHasFile(bool has_file) { has_file_ = has_file; }
  void setHasThread(bool has_thread) { has_thread_ = has_thread; }
  void setClassFuncFormat(ClassFuncFormat format)
  {
    class_func_format_ = format;
  }

protected:
  bool has_date_{ true };
  bool has_thread_{ true };
  bool has_file_{ false };

  ClassFuncFormat class_func_format_{ ClassFuncFormat::ClassOnly };
};
} // namespace eldr::core
