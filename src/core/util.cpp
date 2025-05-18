#include <eldr/core/util.hpp>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

namespace eldr::core::util {
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

} // namespace eldr::core::util
