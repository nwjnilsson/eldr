#pragma once
#include <iostream>
#include <string>
namespace eldr {

// Useful for creating class invariants. Consider the lambda expression below.
// If x is a function parameter that needs to be less than y, take some action
// using the error code 'type'
// expect([x, this] { return x < y; } ErrorCode::type);

enum class ErrorAction { ignore, throwing, terminate, log };

enum class ErrorCode { range_error, length_error };

const std::string error_code_name[]{ "range error", "length error" };

constexpr ErrorAction default_error_action = ErrorAction::throwing;

template <ErrorAction action = default_error_action, class C>
constexpr void expect(C cond, ErrorCode code)
{
  if constexpr (action == ErrorAction::log)
    if (!cond())
      std::cerr << "expect() failure: " << int(code) << " -> "
                << error_code_name[int(code)] << '\n';
  if constexpr (action == ErrorAction::throwing)
    if (!cond()) throw code;
  if constexpr (action == ErrorAction::terminate)
    if (!cond()) std::terminate();

  // or ignore, i.e no code is generated

}

} // namespace eldr
