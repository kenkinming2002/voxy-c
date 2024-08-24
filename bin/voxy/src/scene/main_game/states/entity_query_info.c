#include "entity_query_info.h"

struct entity_query_info *entity_query_info_create(void)
{
  struct entity_query_info *entity_query_info = malloc(sizeof *entity_query_info);
  DYNAMIC_ARRAY_INIT(entity_query_info->entities);
  return entity_query_info;
}


void entity_query_info_destroy(struct entity_query_info *entity_query_info)
{
  free(entity_query_info->entities.items);
  free(entity_query_info);
}

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX entity_query_info
#define SC_HASH_TABLE_NODE_TYPE struct entity_query_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t entity_query_info_key(struct entity_query_info *entity_query_info)
{
  return entity_query_info->position;
}

size_t entity_query_info_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int entity_query_info_compare(ivec3_t position1, ivec3_t position2)
{
  return ivec3_compare(position1, position2);
}

void entity_query_info_dispose(struct entity_query_info *entity_query_info)
{
  entity_query_info_destroy(entity_query_info);
}

