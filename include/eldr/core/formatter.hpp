#pragma once
#include <eldr/core/fwd.hpp>

#include <spdlog/fwd.h>

#include <memory>

namespace eldr::core::logging {
class Formatter {
  friend Logger;

public:
  virtual ~Formatter();

protected:
  Formatter();

protected:
  std::unique_ptr<spdlog::formatter> formatter_;
};

class DefaultFormatter : public Formatter {
public:
  DefaultFormatter();
};

} // namespace eldr::core::logging
