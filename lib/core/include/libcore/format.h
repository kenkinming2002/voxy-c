#ifndef LIBCORE_FORMAT_H
#define LIBCORE_FORMAT_H

#include <stdio.h>
#include <stdlib.h>

#define tformat(fmt, ...) \
  ({ \
    int n = snprintf(NULL, 0, fmt, ##__VA_ARGS__); \
    char *buf = alloca(n + 1); \
    snprintf(buf, n + 1, fmt, ##__VA_ARGS__); \
    buf; \
  })

#define aformat(fmt, ...) \
  ({ \
    int n = snprintf(NULL, 0, fmt, ##__VA_ARGS__); \
    char *buf = malloc(n + 1); \
    snprintf(buf, n + 1, fmt, ##__VA_ARGS__); \
    buf; \
  })

#endif // LIBCORE_FORMAT_H
