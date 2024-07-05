#include <voxy/scene/main_game/types/chunk.h>

struct block *chunk_get_block(struct chunk *chunk, ivec3_t position)
{
  if(position.z < 0) return chunk->bottom ? chunk_get_block(chunk->bottom, ivec3_add(position, ivec3(0, 0, CHUNK_WIDTH))) : NULL;
  if(position.y < 0) return chunk->back   ? chunk_get_block(chunk->back,   ivec3_add(position, ivec3(0, CHUNK_WIDTH, 0))) : NULL;
  if(position.x < 0) return chunk->left   ? chunk_get_block(chunk->left,   ivec3_add(position, ivec3(CHUNK_WIDTH, 0, 0))) : NULL;

  if(position.z >= CHUNK_WIDTH) return chunk->top   ? chunk_get_block(chunk->top,   ivec3_sub(position, ivec3(0, 0, CHUNK_WIDTH))) : NULL;
  if(position.y >= CHUNK_WIDTH) return chunk->front ? chunk_get_block(chunk->front, ivec3_sub(position, ivec3(0, CHUNK_WIDTH, 0))) : NULL;
  if(position.x >= CHUNK_WIDTH) return chunk->right ? chunk_get_block(chunk->right, ivec3_sub(position, ivec3(CHUNK_WIDTH, 0, 0))) : NULL;

  return chunk->data ? chunk_data_get_block(chunk->data, position) : NULL;
}

struct entity *chunk_add_entity(struct chunk *chunk, struct entity entity)
{
  return chunk->data ? chunk_data_add_entity(chunk->data, entity) : NULL;
}
