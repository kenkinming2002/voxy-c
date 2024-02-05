#include "world.h"

#include <stdlib.h>
#include <stdio.h>

int mini(int a, int b)
{
  return a < b ? a : b;
}

void world_update_generate(struct world *world, struct world_generator *world_generator, struct resource_pack *resource_pack)
{
  if(!world->player.spawned)
  {
    world->player.spawned = true;
    world->player.transform.translation = world_generator_generate_spawn(world_generator, resource_pack);
    world->player.transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);

    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        world->player.inventory.items[j][i].id    = ITEM_NONE;
        world->player.inventory.items[j][i].count = 0;
      }

    for(int i=0; i<HOTBAR_SIZE; ++i)
    {
      world->player.hotbar.items[i].id    = ITEM_NONE;
      world->player.hotbar.items[i].count = 0;
    }

    world->player.inventory.opened = false;
    world->player.hotbar.selection = 0;
    world->player.item_hovered = NULL;
    world->player.item_held.id = ITEM_NONE;

    int count = mini(resource_pack->item_info_count * 2, HOTBAR_SIZE);
    for(int i=0; i<count; ++i)
    {
      world->player.hotbar.items[i].id = i % resource_pack->item_info_count;
      world->player.hotbar.items[i].count = 8 * (i + 1);
    }

    world->player.cooldown = 0.0f;

    printf("Spawning player at (%f, %f, %f) with %d items\n",
        world->player.transform.translation.x,
        world->player.transform.translation.y,
        world->player.transform.translation.z,
        count);
  }

  struct chunk      *chunk;
  struct chunk_data *chunk_data;

  ivec3_t player_position = fvec3_as_ivec3_floor(fvec3_div_scalar(world->player.transform.translation, CHUNK_WIDTH));
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        ivec3_t position = ivec3_add(player_position, ivec3(dx, dy, dz));
        if(!(chunk = chunk_hash_table_lookup(&world->chunks, position)) && (chunk_data = world_generator_generate_chunk_data(world_generator, position, resource_pack))) // :)
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
