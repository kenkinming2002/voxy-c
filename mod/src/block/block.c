#include "block.h"

#include "entity/item/item.h"

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

#include <stdlib.h>

static inline float rand_range(float a, float b)
{
  return a + ((float)rand() / RAND_MAX) * (b - a);
}

static inline fvec3_t random_velocity(void)
{
  fvec3_t result;
  do {
    result.x = rand_range(-1.0f, 1.0f);
    result.y = rand_range(-1.0f, 1.0f);
  } while(result.x * result.x + result.y * result.y > 1.0f);
  result.z = rand_range(0.0f, 3.0f);
  return fvec3_mul_scalar(result, 0.2f);
}

void block_on_destroy_spawn_item(struct chunk *chunk, ivec3_t position, item_id_t item_id)
{
  struct entity item_entity;
  item_entity.position = ivec3_as_fvec3(local_position_to_global_position_i(position, chunk->position));
  item_entity.velocity = random_velocity();
  item_entity.rotation = fvec3_zero();
  item_entity.remove = false;
  item_entity.grounded = false;
  item_entity.max_height = -INFINITY;
  item_entity_init(&item_entity, (struct item) { .id = item_id, .count = 1 });
  world_add_entity(item_entity);
}
