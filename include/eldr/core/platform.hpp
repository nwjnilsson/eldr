#pragma once

namespace eldr {
// Reduce namespace pollution from windows.h
#if defined(_WIN32)
#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#  endif
#  if !defined(_USE_MATH_DEFINES)
#    define _USE_MATH_DEFINES
#  endif
#endif

// Likely/unlikely macros (only on GCC/Clang)
#if defined(__GNUG__) || defined(__clang__)
#  define likely(x) __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else
#  define likely(x) (x)
#  define unlikely(x) (x)
#endif

// Processor architecture
#if defined(_MSC_VER) && defined(_M_X86)
#  error 32-bit builds are not supported.
#endif

#if defined(_MSC_VER) // warning C4127: conditional expression is constant
#  pragma warning(disable : 4127)
#endif

} // namespace eldr
