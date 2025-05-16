#include <eldr/core/fwd.hpp>

namespace eldr::core {
class Thread {
public:
  Logger*        logger();
  static Thread* thread();
};
} // namespace eldr::core
