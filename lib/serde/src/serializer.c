#include <libserde/serializer.h>

#include "checksum.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

struct libserde_serializer
{
  FILE *file;
  checksum_t checksum;
};

libserde_serializer_t libserde_serializer_create(const char *path)
{
  libserde_serializer_t serializer = malloc(sizeof *serializer);
  if(!serializer)
    goto error1;

  if(!(serializer->file = fopen(path, "wb")))
    goto error2;

  serializer->checksum = checksum_init();
  return serializer;

error2:
  free(serializer);
error1:
  return NULL;
}

libserde_serializer_t libserde_serializer_create_exclusive(const char *path, int *exist)
{
  libserde_serializer_t serializer = malloc(sizeof *serializer);
  if(!serializer)
  {
    *exist = 0;
    goto error1;
  }

  if(!(serializer->file = fopen(path, "wxb")))
  {
    *exist = errno == EEXIST;
    goto error2;
  }

  serializer->checksum = checksum_init();
  return serializer;

error2:
  free(serializer);
error1:
  return NULL;
}

void libserde_serializer_destroy(libserde_serializer_t serializer)
{
  fclose(serializer->file);
  free(serializer);
}

static int libserde_serializer_write_raw(libserde_serializer_t serializer, const void *data, size_t length)
{
  if(fwrite(data, length, 1, serializer->file) != 1)
    return -1;

  return 0;
}

int libserde_serializer_write(libserde_serializer_t serializer, const void *data, size_t length)
{
  if(libserde_serializer_write_raw(serializer, data, length) != 0)
    return -1;

  serializer->checksum = checksum_update(serializer->checksum, data, length);
  return 0;
}

int libserde_serializer_write_checksum(libserde_serializer_t serializer)
{
  if(libserde_serializer_write_raw(serializer, &serializer->checksum, sizeof serializer->checksum) != 0)
    return -1;

  serializer->checksum = checksum_init();
  return 0;
}
