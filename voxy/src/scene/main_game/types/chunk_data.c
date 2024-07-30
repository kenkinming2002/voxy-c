#include <voxy/scene/main_game/types/chunk_data.h>
#include <voxy/scene/main_game/types/registry.h>

#include <assert.h>
#include <stdlib.h>

static void entities_destroy(struct entities *entities)
{
  for(size_t i=0; i<entities->item_count; ++i)
  {
    struct entity *entity = &entities->items[i];
    const struct entity_info *entity_info = query_entity_info(entity->id);
    if(entity_info->on_dispose)
      entity_info->on_dispose(entity);
  }
  DYNAMIC_ARRAY_CLEAR(*entities);
}

void chunk_data_destroy(struct chunk_data *chunk_data)
{
  entities_destroy(&chunk_data->entities);
  entities_destroy(&chunk_data->new_entities);
  free(chunk_data);
}

struct block *chunk_data_get_block(struct chunk_data *chunk_data, ivec3_t position)
{
  assert(0 <= position.x && position.x < CHUNK_WIDTH);
  assert(0 <= position.y && position.y < CHUNK_WIDTH);
  assert(0 <= position.z && position.z < CHUNK_WIDTH);
  return &chunk_data->blocks[position.z][position.y][position.x];
}

void chunk_data_add_entity(struct chunk_data *chunk_data, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(chunk_data->new_entities, entity);
}

void chunk_data_add_entity_raw(struct chunk_data *chunk_data, struct entity entity)
{
  DYNAMIC_ARRAY_APPEND(chunk_data->entities, entity);
}

void chunk_data_commit_add_entities(struct chunk_data *chunk_data)
{
  DYNAMIC_ARRAY_APPEND_MANY(chunk_data->entities, chunk_data->new_entities.items, chunk_data->new_entities.item_count);
  DYNAMIC_ARRAY_CLEAR(chunk_data->new_entities);
}
