#ifndef LIBCOMMON_CORE_SERDE_H
#define LIBCOMMON_CORE_SERDE_H

#include <stdio.h>

struct serializer
{
  FILE *file;
};

struct deserializer
{
  FILE *file;
};

int serializer_init(struct serializer *serializer, const char *filename);
void serializer_fini(struct serializer *serializer);
int serializer_write(struct serializer *serializer, const void *data, size_t length);

int deserializer_init(struct deserializer *deserializer, const char *filename);
void deserializer_fini(struct deserializer *deserializer);
int deserializer_read(struct deserializer *deserializer, void *data, size_t length);

#define SERIALIZE(serializer, item) do { if(serializer_write(serializer, &item, sizeof item) != 0) { return -1; } } while(0)
#define DESERIALIZE(deserializer, item) do { if(deserializer_read(deserializer, &item, sizeof item) != 0) return -1; } while(0)

#endif // LIBCOMMON_CORE_SERDE_H
