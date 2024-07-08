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

void chunk_data_add_entity(struct chunk_data *chunk_data, struct entity entity)
{
  if(chunk_data->new_entity_capacity == chunk_data->new_entity_count)
  {
    chunk_data->new_entity_capacity = chunk_data->new_entity_capacity != 0 ? chunk_data->new_entity_capacity * 2 : 1;
    chunk_data->new_entities = realloc(chunk_data->new_entities, chunk_data->new_entity_capacity * sizeof *chunk_data->new_entities);
  }
  chunk_data->new_entities[chunk_data->new_entity_count++] = entity;
}

void chunk_data_commit_add_entities(struct chunk_data *chunk_data)
{
  while(chunk_data->entity_capacity < chunk_data->entity_count + chunk_data->new_entity_count)
    chunk_data->entity_capacity = chunk_data->entity_capacity != 0 ? chunk_data->entity_capacity * 2 : 1;
  chunk_data->entities = realloc(chunk_data->entities, chunk_data->entity_capacity * sizeof *chunk_data->entities);

  for(size_t i=0; i<chunk_data->new_entity_count; ++i)
    chunk_data->entities[chunk_data->entity_count++] = chunk_data->new_entities[i];
  chunk_data->new_entity_count = 0;
}
