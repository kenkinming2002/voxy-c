#include <libserde/deserializer.h>

#include "checksum.h"

#include <stdio.h>
#include <stdlib.h>

struct libserde_deserializer
{
  FILE *file;
  checksum_t checksum;
};

libserde_deserializer_t libserde_deserializer_create(const char *path)
{
  libserde_deserializer_t deserializer = malloc(sizeof *deserializer);
  if(!deserializer)
    return NULL;

  if(!(deserializer->file = fopen(path, "r")))
    return NULL;

  deserializer->checksum = checksum_init();
  return deserializer;
}

void libserde_deserializer_destroy(libserde_deserializer_t deserializer)
{
  fclose(deserializer->file);
  free(deserializer);
}

static int libserde_deserializer_read_raw(libserde_deserializer_t deserializer, void *data, size_t length)
{
  if(fread(data, length, 1, deserializer->file) != 1)
    return -1;

  return 0;
}

int libserde_deserializer_read(libserde_deserializer_t deserializer, void *data, size_t length)
{
  if(libserde_deserializer_read_raw(deserializer, data, length) != 0)
    return -1;

  deserializer->checksum = checksum_update(deserializer->checksum, data, length);
  return 0;
}

int libserde_deserializer_read_checksum(libserde_deserializer_t deserializer)
{
  checksum_t checksum;
  if(libserde_deserializer_read_raw(deserializer, &checksum, sizeof checksum) != 0)
    return -1;

  if(checksum != deserializer->checksum)
    return -1;

  deserializer->checksum = checksum_init();
  return 0;
}

