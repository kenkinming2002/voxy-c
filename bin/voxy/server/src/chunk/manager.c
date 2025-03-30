#include "manager.h"

#include <stb_ds.h>

void voxy_chunk_manager_init(struct voxy_chunk_manager *chunk_manager)
{
  chunk_manager->active_chunks = NULL;
}

void voxy_chunk_manager_fini(struct voxy_chunk_manager *chunk_manager)
{
  hmfree(chunk_manager->active_chunks);
}

void voxy_chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager)
{
  hmfree(chunk_manager->active_chunks);
}

void voxy_chunk_manager_add_active_chunk(struct voxy_chunk_manager *chunk_manager, ivec3_t position)
{
  hmput(chunk_manager->active_chunks, position, (struct empty){});
}

