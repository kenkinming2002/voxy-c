#include "world.h"

#include "world_generator.h"

#include <stdlib.h>

void world_update_chunk_generate(struct world *world, struct resource_pack *resource_pack, struct world_generator *world_generator)
{
  struct chunk      *chunk;
  struct chunk_data *chunk_data;

  ivec3_t player_position = fvec3_as_ivec3_floor(fvec3_div_scalar(world->player.base.position, CHUNK_WIDTH));
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        ivec3_t position = ivec3_add(player_position, ivec3(dx, dy, dz));
        if(!(chunk = chunk_hash_table_lookup(&world->chunks, position)) && (chunk_data = world_generator_generate_chunk_data(world_generator, world, position, resource_pack))) // :)
        {
          chunk = malloc(sizeof *chunk);
          chunk->position   = position;
          chunk->chunk_data = chunk_data;
          chunk->chunk_mesh = NULL;

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
