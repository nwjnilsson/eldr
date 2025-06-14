#include <eldr/core/logger.hpp>
#include <eldr/core/util.hpp>
#include <eldr/eldr.hpp>

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

NAMESPACE_END(eldr::util)
