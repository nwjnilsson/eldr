#pragma once
#include <eldr.hpp>

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
#  define EL_X86_64 1
#endif

#if (defined(__i386__) || defined(_M_IX86)) && !defined(EL_X86_64)
#  define EL_X86_32 1
#endif

#if defined(__aarch64__)
#  define EL_ARM_64 1
#elif defined(__arm__)
#  define EL_ARM_32 1
#endif

// SIMD instruction set detection
// These are defined based on compiler-provided macros when -march=native is
// used
#if !defined(EL_DISABLE_SIMD)
#  if defined(__AVX512F__) && defined(__AVX512CD__) &&                         \
    defined(__AVX512VL__) && defined(__AVX512DQ__) && defined(__AVX512BW__)
#    define EL_X86_AVX512 1
#  endif
#  if defined(__AVX512VBMI__)
#    define EL_X86_AVX512VBMI 1
#  endif
#  if defined(__AVX512VPOPCNTDQ__)
#    define EL_X86_AVX512VPOPCNTDQ 1
#  endif
#  if defined(__AVX2__)
#    define EL_X86_AVX2 1
#  endif
#  if defined(__FMA__)
#    define EL_X86_FMA 1
#  endif
#  if defined(__F16C__)
#    define EL_X86_F16C 1
#  endif
#  if defined(__BMI__)
#    define EL_X86_BMI 1
#  endif
#  if defined(__BMI2__)
#    define EL_X86_BMI2 1
#  endif
#  if defined(__AVX__)
#    define EL_X86_AVX 1
#  endif
#  if defined(__SSE4_2__)
#    define EL_X86_SSE42 1
#  endif
#  if defined(__ARM_NEON)
#    define EL_ARM_NEON 1
#  endif
#  if defined(__ARM_FEATURE_FMA)
#    define EL_ARM_FMA 1
#  endif
#endif

NAMESPACE_BEGIN(eldr::embr)

// Default SIMD packet size based on available instruction sets
#if defined(EL_X86_AVX512)
constexpr size_t kDefaultSize{ 16 };
#elif defined(EL_X86_AVX) || defined(EL_X86_AVX2)
constexpr size_t kDefaultSize{ 8 };
#elif defined(EL_X86_SSE42)
constexpr size_t kDefaultSize{ 4 };
#elif defined(EL_ARM_NEON)
constexpr size_t kDefaultSize{ 4 };
#else
constexpr size_t kDefaultSize{ 1 };
#endif

template <typename _Val, size_t _Sz = kDefaultSize> struct Packet;
template <typename _Val, size_t _Sz> struct Matrix;
template <typename _Float> struct Quaternion;
NAMESPACE_END(eldr::embr)
