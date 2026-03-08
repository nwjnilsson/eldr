#pragma once
#include "fwd.hpp"

NAMESPACE_BEGIN(eldr::embr)

// -----------------------------------------------------------------------------
// Available instruction sets (compile-time constants)
// Matching DrJit's packet_intrin.h pattern
// -----------------------------------------------------------------------------

#if defined(EL_X86_32)
static constexpr bool has_x86_32 = true;
#else
static constexpr bool has_x86_32 = false;
#endif

#if defined(EL_X86_64)
static constexpr bool has_x86_64 = true;
#else
static constexpr bool has_x86_64 = false;
#endif

#if defined(EL_ARM_32)
static constexpr bool has_arm_32 = true;
#else
static constexpr bool has_arm_32 = false;
#endif

#if defined(EL_ARM_64)
static constexpr bool has_arm_64 = true;
#else
static constexpr bool has_arm_64 = false;
#endif

#if defined(EL_X86_SSE42)
static constexpr bool has_sse42 = true;
#else
static constexpr bool has_sse42 = false;
#endif

#if defined(EL_X86_FMA) || defined(EL_ARM_FMA)
static constexpr bool has_fma = true;
#else
static constexpr bool has_fma = false;
#endif

#if defined(EL_X86_F16C)
static constexpr bool has_f16c = true;
#else
static constexpr bool has_f16c = false;
#endif

#if defined(EL_X86_AVX)
static constexpr bool has_avx = true;
#else
static constexpr bool has_avx = false;
#endif

#if defined(EL_X86_BMI)
static constexpr bool has_bmi = true;
#else
static constexpr bool has_bmi = false;
#endif

#if defined(EL_X86_BMI2)
static constexpr bool has_bmi2 = true;
#else
static constexpr bool has_bmi2 = false;
#endif

#if defined(EL_X86_AVX2)
static constexpr bool has_avx2 = true;
#else
static constexpr bool has_avx2 = false;
#endif

#if defined(EL_X86_AVX512)
static constexpr bool has_avx512 = true;
#else
static constexpr bool has_avx512 = false;
#endif

#if defined(EL_X86_AVX512VBMI)
static constexpr bool has_avx512vbmi = true;
#else
static constexpr bool has_avx512vbmi = false;
#endif

#if defined(EL_X86_AVX512VPOPCNTDQ)
static constexpr bool has_avx512vpopcntdq = true;
#else
static constexpr bool has_avx512vpopcntdq = false;
#endif

#if defined(EL_ARM_NEON)
static constexpr bool has_neon = true;
#else
static constexpr bool has_neon = false;
#endif

// Derived constants
static constexpr bool has_x86 = has_x86_32 || has_x86_64;
static constexpr bool has_arm = has_arm_32 || has_arm_64;

NAMESPACE_END(eldr::embr)
