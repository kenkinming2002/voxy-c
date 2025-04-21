#ifndef ABOMINATION_STRING_H
#define ABOMINATION_STRING_H

#include_next <string.h>

static inline void *memcpy_safe(void *restrict dest, const void *restrict src, size_t n)
{
  if(n != 0)
    return memcpy(dest, src, n);
  else
    return dest;
}

#define memcpy memcpy_safe

#endif // ABOMINATION_STRING_H
