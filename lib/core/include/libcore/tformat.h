#ifndef LIBCOMMON_UTILS_TFORMAT_H
#define LIBCOMMON_UTILS_TFORMAT_H

#include <stdio.h>

#define tformat(fmt, ...) \
  ({ \
    int n = snprintf(NULL, 0, fmt, ##__VA_ARGS__); \
    char *buf = alloca(n + 1); \
    snprintf(buf, n + 1, fmt, ##__VA_ARGS__); \
    buf; \
  })

#endif // LIBCOMMON_UTILS_TFORMAT_H
