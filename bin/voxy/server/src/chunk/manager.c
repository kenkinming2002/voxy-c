#include "manager.h"

#include <stdlib.h>

void voxy_chunk_manager_init(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_init(&chunk_manager->active_chunks);
}

void voxy_chunk_manager_fini(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

void voxy_chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

void voxy_chunk_manager_add_active_chunk(struct voxy_chunk_manager *chunk_manager, ivec3_t position)
{
  struct ivec3_node *node;
  if(!(node = ivec3_hash_table_lookup(&chunk_manager->active_chunks, position)))
  {
    node = malloc(sizeof *node);
    node->key = position;
    ivec3_hash_table_insert_unchecked(&chunk_manager->active_chunks, node);
  }
}

