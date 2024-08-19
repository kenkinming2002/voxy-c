#ifndef BLOCK_CHEST_CHEST_H
#define BLOCK_CHEST_CHEST_H

#include <voxy/scene/main_game/types/registry.h>

void chest_block_register(void);
block_id_t chest_block_id_get(void);

void chest_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void chest_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

bool chest_block_on_use(struct entity *entity, struct chunk *chunk, ivec3_t position);

int chest_block_serialize(const void *data, struct serializer *serializer);
int chest_block_deserialize(void **data, struct deserializer *deserializer);

#endif // BLOCK_CHEST_CHEST_H
