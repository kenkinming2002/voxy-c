#include "world.h"

#include <stdlib.h>
#include <stdio.h>

void world_update_generate(struct world *world, struct world_generator *world_generator)
{
  if(!world->player.spawned)
  {
    world->player.spawned = true;
    world->player.transform.translation = fvec3(0.0f, 0.0f, world_generator_get_height(world_generator, ivec2(0, 0)) + 2.0f);
    world->player.transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);
    printf("Spawning player at (%f, %f, %f)\n",
        world->player.transform.translation.x,
        world->player.transform.translation.y,
        world->player.transform.translation.z);
  }

  struct chunk      *chunk;
  struct chunk_data *chunk_data;

  ivec3_t player_position = fvec3_as_ivec3_floor(fvec3_div_scalar(world->player.transform.translation, CHUNK_WIDTH));
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        ivec3_t position = ivec3_add(player_position, ivec3(dx, dy, dz));
        if(!(chunk = chunk_hash_table_lookup(&world->chunks, position)) && (chunk_data = world_generator_generate_chunk_data(world_generator, position))) // :)
        {
          chunk = malloc(sizeof *chunk);
          chunk->position   = position;
          chunk->chunk_data = chunk_data;

          world_chunk_insert_unchecked(world, chunk);

          chunk->mesh_dirty  = true;
          chunk->light_dirty = true;

          if(chunk->left)   chunk->left  ->mesh_dirty = true;
          if(chunk->right)  chunk->right ->mesh_dirty = true;
          if(chunk->back)   chunk->back  ->mesh_dirty = true;
          if(chunk->front)  chunk->front ->mesh_dirty = true;
          if(chunk->bottom) chunk->bottom->mesh_dirty = true;
          if(chunk->top)    chunk->top   ->mesh_dirty = true;
        }
      }
}
