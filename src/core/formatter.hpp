#pragma once
#include <string>

NAMESPACE_BEGIN(eldr)
class Formatter {
  friend class Logger;

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
NAMESPACE_END(eldr)

/// Specialize a simple formatter, without format specifiers and based on
/// util::toString()
#define EL_SPECIALIZE_FORMATTER(Type)                                          \
  template <> struct std::formatter<Type> {                                    \
    constexpr auto parse(std::format_parse_context& ctx)                       \
    {                                                                          \
      return ctx.begin();                                                      \
    }                                                                          \
    EL_INLINE auto format(const Type& v, std::format_context& ctx) const       \
    {                                                                          \
      return std::format_to(ctx.out(), "{}", eldr::util::toString(v));         \
    }                                                                          \
  };

/// Define ostream operator based on util::toString
#define EL_DEFINE_OSTR(Type)                                                   \
  EL_INLINE std::ostream& operator<<(std::ostream& os, const Type& v)          \
  {                                                                            \
    os << util::toString(v);                                                   \
    return os;                                                                 \
  }
