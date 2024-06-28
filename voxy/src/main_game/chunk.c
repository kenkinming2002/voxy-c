#include <voxy/main_game/chunk.h>

#include <stdlib.h>

struct block *chunk_get_block(struct chunk *chunk, ivec3_t position)
{
  if(!chunk)
    return NULL;

  if(position.z >= 0 && position.z < CHUNK_WIDTH)
    if(position.y >= 0 && position.y < CHUNK_WIDTH)
      if(position.x >= 0 && position.x < CHUNK_WIDTH)
        return &chunk->blocks[position.z][position.y][position.x];

  if(position.x == -1)          return chunk_get_block(chunk->left,   ivec3_add(position, ivec3(CHUNK_WIDTH, 0, 0)));
  if(position.x == CHUNK_WIDTH) return chunk_get_block(chunk->right,  ivec3_sub(position, ivec3(CHUNK_WIDTH, 0, 0)));
  if(position.y == -1)          return chunk_get_block(chunk->back,   ivec3_add(position, ivec3(0, CHUNK_WIDTH, 0)));
  if(position.y == CHUNK_WIDTH) return chunk_get_block(chunk->front,  ivec3_sub(position, ivec3(0, CHUNK_WIDTH, 0)));
  if(position.z == -1)          return chunk_get_block(chunk->bottom, ivec3_add(position, ivec3(0, 0, CHUNK_WIDTH)));
  if(position.z == CHUNK_WIDTH) return chunk_get_block(chunk->top,    ivec3_sub(position, ivec3(0, 0, CHUNK_WIDTH)));

  assert(0 && "Unreachable");
}

void chunk_add_entity(struct chunk *chunk, struct entity entity)
{
  if(chunk->entity_capacity == chunk->entity_count)
  {
    chunk->entity_capacity = chunk->entity_capacity != 0 ? chunk->entity_capacity * 2 : 1;
    chunk->entities        = realloc(chunk->entities, chunk->entity_capacity * sizeof *chunk->entities);
  }
  chunk->entities[chunk->entity_count++] = entity;
}
