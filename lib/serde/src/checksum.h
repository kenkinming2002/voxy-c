#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t checksum_t;

static inline checksum_t checksum_init(void)
{
  return 0;
}

static inline checksum_t checksum_update(checksum_t checksum, const void *data, size_t length)
{
  for(size_t i=0; i<length; ++i)
    checksum ^= ((const uint8_t *)data)[i];
  return checksum;
}

#endif // CHECKSUM_H
