#ifndef VOXY_HASH_H
#define VOXY_HASH_H

#include <stddef.h>

static inline size_t hash2(int x, int y)
{
  return x * 23 + y * 31;
}

static inline size_t hash3(int x, int y, int z)
{
  return x * 23 + y * 31 + z * 41;
}

#endif // VOXY_HASH_H
