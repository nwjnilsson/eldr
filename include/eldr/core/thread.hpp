#pragma once
#include <eldr/core/fwd.hpp>

#include <memory>
#include <string>

namespace eldr::core {
/// Mock thread class for now until I have time to implement it
class Thread {
public:
  Thread(std::string_view name) : name_(name) {};

  virtual ~Thread() = default;

public:
  void setLogger(std::unique_ptr<Logger> logger)
  {
    logger_ = std::move(logger);
  }

  [[nodiscard]] const std::string& name() const { return name_; }

  [[nodiscard]] Logger* logger() { return logger_.get(); }

  [[nodiscard]] static Thread* thread();

  static void createContext();

protected:
  virtual void run() = 0;

private:
  std::string             name_;
  std::unique_ptr<Logger> logger_;
};
} // namespace eldr::core
