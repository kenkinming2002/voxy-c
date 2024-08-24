#ifndef SCENE_MAIN_GAME_STATES_ENTITY_QUERY_INFO_H
#define SCENE_MAIN_GAME_STATES_ENTITY_QUERY_INFO_H

#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>

DYNAMIC_ARRAY_DEFINE(p_entities, struct entity *);

struct entity_query_info
{
  struct entity_query_info *next;
  size_t                    hash;

  ivec3_t position;
  struct p_entities entities;
};

struct entity_query_info *entity_query_info_create(void);
void entity_query_info_destroy(struct entity_query_info *entity_query_info);

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX entity_query_info
#define SC_HASH_TABLE_NODE_TYPE struct entity_query_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_INTERFACE

ivec3_t entity_query_info_key(struct entity_query_info *entity_query_info);
size_t entity_query_info_hash(ivec3_t position);
int entity_query_info_compare(ivec3_t position1, ivec3_t position2);
void entity_query_info_dispose(struct entity_query_info *entity_query_info);

#endif // SCENE_MAIN_GAME_STATES_ENTITY_QUERY_INFO_H
