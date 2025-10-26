#pragma once
#include <cstdlib>
#if defined(_MSC_VER)
#  include <stdio.h>
#  define arr_fail(...)                                                        \
    do {                                                                       \
      printf(__VA_ARGS__);                                                     \
      abort();                                                                 \
    } while (0)
#else
#  define arr_fail(...)                                                        \
    do {                                                                       \
      __builtin_printf(__VA_ARGS__);                                           \
      abort();                                                                 \
    } while (0)
#endif
