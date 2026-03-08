#include <core/logger.hpp>
#include <core/util.hpp>
#include <eldr.hpp>
#include <embr/packet.hpp>

#include <sstream>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

NAMESPACE_BEGIN(eldr::util)

std::string infoBuild(int thread_count)
{
  std::ostringstream oss;
  oss << EL_NAME << " version " << EL_VERSION_STR ", (";
#if defined(_WIN32)
  oss << "Windows, ";
#elif defined(__linux__)
  oss << "Linux, ";
#elif defined(__APPLE__)
  oss << "Mac OS, ";
#else
  oss << "Unknown, ";
#endif
  oss << (sizeof(size_t) * 8) << "bit, ";
  oss << thread_count << " thread" << (thread_count > 1 ? "s" : "");
  oss << ", " << em::Packet<float>::kSize << "-wide SIMD";
  oss << ")";
  return oss.str();
}

std::string infoCopyright()
{
  std::ostringstream oss;
  oss << "Copyright " << EL_YEAR << ", " << EL_AUTHORS;
  return oss.str();
}

int terminalWidth()
{
  int width{ -1 };
#ifdef _WIN32
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  if (h != INVALID_HANDLE_VALUE && h != nullptr) {
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
    GetConsoleScreenBufferInfo(h, &bufferInfo);
    width = bufferInfo.dwSize.X - 1;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) >= 0)
    width = w.ws_col;
#endif
  if (width == -1)
    width = 80;
  return width;
}

std::filesystem::path eldrRootDir()
{
  const char* root = std::getenv("ELDR_DIR");
  if (root == nullptr) {
    Throw("Environment not set up correctly");
  }
  return std::filesystem::path(root);
}

std::string infoFeatures()
{
  std::ostringstream oss;
  bool               first = true;

  auto add_feature = [&](const char* name) {
    if (!first)
      oss << " ";
    oss << name;
    first = false;
  };

  // Architecture
  if constexpr (embr::has_x86_64)
    add_feature("x86_64");
  else if constexpr (embr::has_x86_32)
    add_feature("x86_32");
  else if constexpr (embr::has_arm_64)
    add_feature("arm_64");
  else if constexpr (embr::has_arm_32)
    add_feature("arm_32");

  // x86 instruction sets
  if constexpr (embr::has_sse42)
    add_feature("sse4.2");
  if constexpr (embr::has_avx)
    add_feature("avx");
  if constexpr (embr::has_avx2)
    add_feature("avx2");
  if constexpr (embr::has_fma)
    add_feature("fma");
  if constexpr (embr::has_f16c)
    add_feature("f16c");
  if constexpr (embr::has_bmi)
    add_feature("bmi");
  if constexpr (embr::has_bmi2)
    add_feature("bmi2");
  if constexpr (embr::has_avx512)
    add_feature("avx512");
  if constexpr (embr::has_avx512vbmi)
    add_feature("avx512vbmi");
  if constexpr (embr::has_avx512vpopcntdq)
    add_feature("avx512vpopcntdq");

  // ARM instruction sets
  if constexpr (embr::has_neon)
    add_feature("neon");

#if defined(EL_DISABLE_SIMD)
  if (first) {
    oss << "simd_disabled";
  }
  else {
    oss << " simd_disabled";
  }
#endif

  return oss.str();
}

NAMESPACE_END(eldr::util)
