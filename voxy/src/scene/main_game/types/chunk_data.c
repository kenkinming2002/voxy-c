#include <voxy/scene/main_game/types/chunk_data.h>

#include <assert.h>
#include <stdlib.h>

struct block *chunk_data_get_block(struct chunk_data *chunk_data, ivec3_t position)
{
  assert(0 <= position.x && position.x < CHUNK_WIDTH);
  assert(0 <= position.y && position.y < CHUNK_WIDTH);
  assert(0 <= position.z && position.z < CHUNK_WIDTH);
  return &chunk_data->blocks[position.z][position.y][position.x];
}

struct entity *chunk_data_add_entity(struct chunk_data *chunk_data, struct entity entity)
{
  if(chunk_data->entity_capacity == chunk_data->entity_count)
  {
    chunk_data->entity_capacity = chunk_data->entity_capacity != 0 ? chunk_data->entity_capacity * 2 : 1;
    chunk_data->entities = realloc(chunk_data->entities, chunk_data->entity_capacity * sizeof *chunk_data->entities);
  }
  chunk_data->entities[chunk_data->entity_count++] = entity;
  return &chunk_data->entities[chunk_data->entity_count-1];
}
