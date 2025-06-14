#pragma once
#include <eldr/core/logger.hpp>

/// Currently not in use anywhere. TODO: improve or delete

NAMESPACE_BEGIN(eldr)

// Useful for creating class invariants. Consider the lambda expression below.
// If x is a function parameter that needs to be less than y, take some action
// using the error code 'type'
// expect([x, this] { return x < y; } ErrorCode::type);

enum class ErrorAction { Ignore, Throwing, Terminate, Log };

enum class ErrorCode { RangeError, LengthError, ValueError };

const std::string error_code_name[]{ "range error",
                                     "length error",
                                     "value error" };

constexpr ErrorAction default_error_action{ ErrorAction::Throwing };

template <ErrorAction action = default_error_action, class C>
constexpr void expect(C cond, ErrorCode code)
{
  if constexpr (action == ErrorAction::Log)
    if (!cond())
      Log(Error,
          "expect() failure: {} ->",
          static_cast<int>(code),
          error_code_name[static_cast<int>(code)]);
  if constexpr (action == ErrorAction::Throwing)
    if (!cond())
      Throw("{}", error_code_name[static_cast<int>(code)]);
  if constexpr (action == ErrorAction::Terminate)
    if (!cond())
      std::terminate();

  // or ignore, i.e no code is generated
}

NAMESPACE_END(eldr)
