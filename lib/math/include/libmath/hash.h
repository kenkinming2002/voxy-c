#ifndef LIBMATH_HASH_H
#define LIBMATH_HASH_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#define FNV_DEFINE(type, basis, prime) \
  static inline type hash_##type##_fnv_1(void *data, size_t length)  \
  {                                                                  \
    type hash = basis;                                               \
                                                                     \
    char *_data = data;                                              \
    for(size_t i=0; i<length; ++i)                                   \
    {                                                                \
      hash = hash * prime;                                           \
      hash = hash ^ _data[i];                                        \
    }                                                                \
                                                                     \
    return hash;                                                     \
  }                                                                  \
                                                                     \
  static inline type hash_##type##_fnv_1a(void *data, size_t length) \
  {                                                                  \
    type hash = basis;                                               \
                                                                     \
    char *_data = data;                                              \
    for(size_t i=0; i<length; ++i)                                   \
    {                                                                \
      hash = hash ^ _data[i];                                        \
      hash = hash * prime;                                           \
    }                                                                \
                                                                     \
    return hash;                                                     \
  }                                                                  \

FNV_DEFINE(uint32_t, 0x811c9dc5,         0x01000193)
FNV_DEFINE(uint64_t, 0xcbf29ce484222325, 0x00000100000001b3)

static inline size_t hash_fnv_1(void *data, size_t length)
{
  static_assert(sizeof(size_t) == sizeof(uint32_t) || sizeof(size_t) == sizeof(uint64_t),  "WTH");
  if(sizeof(size_t) == sizeof(uint32_t)) return hash_uint32_t_fnv_1(data, length);
  if(sizeof(size_t) == sizeof(uint64_t)) return hash_uint64_t_fnv_1(data, length);
}

static inline size_t hash_fnv_1a(void *data, size_t length)
{
  static_assert(sizeof(size_t) == sizeof(uint32_t) || sizeof(size_t) == sizeof(uint64_t),  "WTH");
  if(sizeof(size_t) == sizeof(uint32_t)) return hash_uint32_t_fnv_1a(data, length);
  if(sizeof(size_t) == sizeof(uint64_t)) return hash_uint64_t_fnv_1a(data, length);
}

#endif // LIBMATH_HASH_H
