#include "spawn_weird.h"
#include "../entity/weird/weird.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <stdlib.h>

static float randf(float a, float b)
{
  return a + (float)rand() / (float)RAND_MAX * (b - a);
}


void spawn_weird_update(void)
{
  world_for_each_chunk(chunk)
    if(randf(0.0f, 1.0f) < 1e-6)
    {
      fvec3_t local_position;
      local_position.x = randf(0.0f, CHUNK_WIDTH);
      local_position.y = randf(0.0f, CHUNK_WIDTH);
      local_position.z = randf(0.0f, CHUNK_WIDTH);

      fvec3_t position = local_position_to_global_position_f(local_position, chunk->position);

      struct entity entity;
      entity.position = position;
      entity.velocity = fvec3_zero();
      entity.rotation = fvec3_zero();
      entity.remove = false;
      entity.grounded = false;
      entity.max_height = -INFINITY;
      weird_entity_init(&entity);
      chunk_add_entity(chunk, entity);
      return;
    }
}
