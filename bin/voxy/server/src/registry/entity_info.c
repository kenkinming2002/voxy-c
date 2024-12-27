#include <voxy/server/registry/entity_info.h>

void voxy_entity_destroy_opaque_default(void *opaque)
{
  (void)opaque;
}

int voxy_entity_serialize_opaque_default(libserde_serializer_t serializer, const void *opaque)
{
  (void)serializer;
  (void)opaque;
  return 0;
}

int voxy_entity_deserialize_opaque_default(libserde_deserializer_t deserializer, void **opaque)
{
  (void)deserializer;
  (void)opaque;
  *opaque = NULL;
  return 0;
}

