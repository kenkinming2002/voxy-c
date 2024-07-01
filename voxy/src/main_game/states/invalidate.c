#include <voxy/main_game/states/invalidate.h>

struct chunk *chunks_invalidated_light_head;
struct chunk *chunks_invalidated_light_tail;

struct chunk *chunks_invalidated_mesh_head;
struct chunk *chunks_invalidated_mesh_tail;

void world_invalidate_chunk_light(struct chunk *chunk)
{
  if(chunk && !chunk->light_invalidated)
  {
    chunk->light_invalidated = true;
    chunk->light_next = NULL;
    if(chunks_invalidated_light_tail)
    {
      chunks_invalidated_light_tail->light_next = chunk;
      chunks_invalidated_light_tail = chunk;
    }
    else
    {
      chunks_invalidated_light_head = chunk;
      chunks_invalidated_light_tail = chunk;
    }
  }
}

void world_invalidate_chunk_mesh(struct chunk *chunk)
{
  if(chunk && !chunk->mesh_invalidated)
  {
    chunk->mesh_invalidated = true;
    chunk->mesh_next = NULL;
    if(chunks_invalidated_mesh_tail)
    {
      chunks_invalidated_mesh_tail->mesh_next = chunk;
      chunks_invalidated_mesh_tail = chunk;
    }
    else
    {
      chunks_invalidated_mesh_head = chunk;
      chunks_invalidated_mesh_tail = chunk;
    }
  }
}

