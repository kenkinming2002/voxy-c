#include <voxy/scene/main_game/states/entity_query.h>
#include <voxy/scene/main_game/states/chunks.h>

#include "entity_query_info.h"

static struct entity_query_info_hash_table entity_query_infos;

void world_query_entity_begin(void)
{
  struct chunk *chunk;
  SC_HASH_TABLE_FOREACH(world_chunks, chunk)
    for(size_t i=0; i<chunk->entities.item_count; ++i)
    {
      struct entity *entity = &chunk->entities.items[i];
      const aabb3_t hitbox = entity_hitbox(entity);

      const ivec3_t chunk_position_min = get_chunk_position_f(aabb3_min_corner(hitbox));
      const ivec3_t chunk_position_max = get_chunk_position_f(aabb3_max_corner(hitbox));
      for(int z=chunk_position_min.z; z<=chunk_position_max.z; ++z)
        for(int y=chunk_position_min.y; y<=chunk_position_max.y; ++y)
          for(int x=chunk_position_min.x; x<=chunk_position_max.x; ++x)
          {
            const ivec3_t chunk_position = ivec3(x, y, z);
            struct entity_query_info *entity_query_info = entity_query_info_hash_table_lookup(&entity_query_infos, chunk_position);
            if(!entity_query_info)
            {
              entity_query_info = entity_query_info_create();
              entity_query_info->position = chunk_position;
              entity_query_info_hash_table_insert_unchecked(&entity_query_infos, entity_query_info);
            }
            DYNAMIC_ARRAY_APPEND(entity_query_info->entities, entity);
          }
    }
}

void world_query_entity_end(void)
{
  entity_query_info_hash_table_dispose(&entity_query_infos);
}

void world_query_entity(aabb3_t aabb, struct entity ***entities, size_t *entity_count)
{
  struct p_entities results = {0};

  const ivec3_t chunk_position_min = get_chunk_position_f(aabb3_min_corner(aabb));
  const ivec3_t chunk_position_max = get_chunk_position_f(aabb3_max_corner(aabb));
  for(int z=chunk_position_min.z; z<=chunk_position_max.z; ++z)
    for(int y=chunk_position_min.y; y<=chunk_position_max.y; ++y)
      for(int x=chunk_position_min.x; x<=chunk_position_max.x; ++x)
      {
        const ivec3_t chunk_position = ivec3(x, y, z);
        struct entity_query_info *entity_query_info = entity_query_info_hash_table_lookup(&entity_query_infos, chunk_position);
        if(!entity_query_info)
          continue;

        for(size_t i=0; i<entity_query_info->entities.item_count; ++i)
          if(aabb3_intersect(aabb, entity_hitbox(entity_query_info->entities.items[i])))
            DYNAMIC_ARRAY_APPEND(results, entity_query_info->entities.items[i]);
      }

  *entities = results.items;
  *entity_count = results.item_count;
}
