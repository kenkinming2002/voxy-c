#include "chunk_loader.h"

#include <voxy/core/window.h>
#include <voxy/graphics/camera.h>
#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/update/chunk_generate.h>

void player_entity_update_chunk_loader(struct entity *entity)
{
  ivec3_t center = fvec3_as_ivec3_floor(fvec3_div_scalar(entity->position, CHUNK_WIDTH));
  for(int dz = -PLAYER_CHUNK_LOAD_DISTANCE; dz<=PLAYER_CHUNK_LOAD_DISTANCE; ++dz)
    for(int dy = -PLAYER_CHUNK_LOAD_DISTANCE; dy<=PLAYER_CHUNK_LOAD_DISTANCE; ++dy)
      for(int dx = -PLAYER_CHUNK_LOAD_DISTANCE; dx<=PLAYER_CHUNK_LOAD_DISTANCE; ++dx)
        enqueue_chunk_generate(ivec3_add(center, ivec3(dx, dy, dz)));
}

