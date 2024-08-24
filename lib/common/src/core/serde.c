#include <libcommon/core/serde.h>
#include <libcommon/core/fs.h>

#include <string.h>
#include <stdlib.h>

int serializer_init(struct serializer *serializer, const char *filename)
{
  char *tmp = parent(filename);
  mkdir_recursive(tmp);
  free(tmp);

  serializer->file = fopen(filename, "wb");
  if(!serializer->file)
    return -1;

  return 0;
}

void serializer_fini(struct serializer *serializer)
{
  fclose(serializer->file);
}

int serializer_write(struct serializer *serializer, const void *data, size_t length)
{
  if(fwrite(data, length, 1, serializer->file) != 1)
    return -1;

  return 0;
}

int deserializer_init(struct deserializer *deserializer, const char *filename)
{
  deserializer->file = fopen(filename, "rb");
  if(!deserializer->file)
    return -1;

  return 0;
}

void deserializer_fini(struct deserializer *deserializer)
{
  fclose(deserializer->file);
}

int deserializer_read(struct deserializer *deserializer, void *data, size_t length)
{
  if(fread(data, length, 1, deserializer->file) != 1)
    return -1;

  return 0;
}

