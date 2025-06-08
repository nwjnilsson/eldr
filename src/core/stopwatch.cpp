#include <eldr/core/stopwatch.hpp>

// Keeping this .cpp file for now in case I want to extend the stopwatch class.
// Otherwise everything can be moved to the header.
namespace eldr::core {
StopWatch::StopWatch() : init_(clock::now()), latest_(init_) {}
} // namespace eldr::core
