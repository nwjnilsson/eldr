#include <eldr.hpp>
NAMESPACE_BEGIN(eldr::embr)

#if defined(ELDR_X86_AVX512)
constexpr size_t DefaultSize{ 16 };
#elif defined(ELDR_X86_AVX)
constexpr size_t DefaultSize{ 8 };
#elif defined(ELDR_X86_SSE2)
constexpr size_t DefaultSize{ 4 };
#else
constexpr size_t DefaultSize{ 1 };
#endif
template <typename _Val, size_t _Size = DefaultSize> struct Packet;
template <typename _Val, size_t _Size> struct Matrix;
template <typename _Float> struct Quaternion;
NAMESPACE_END(eldr::embr)
