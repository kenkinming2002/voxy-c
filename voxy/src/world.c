#include "world.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

ivec3_t chunk_key(struct chunk *chunk)
{
  return chunk->position;
}

size_t chunk_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int chunk_compare(ivec3_t position1, ivec3_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_dispose(struct chunk *chunk)
{
  free(chunk);
}

void world_init(struct world *world, seed_t seed)
{
  world->seed = seed;

  chunk_hash_table_init(&world->chunks);

  world->player_transform.translation = fvec3_zero();
  world->player_transform.rotation    = fvec3_zero();
}

void world_deinit(struct world *world)
{
  chunk_hash_table_dispose(&world->chunks);
}

void world_invalidate_chunk_mesh(struct world *world, struct chunk *chunk)
{
  chunk->mesh_dirty = true;

  struct chunk *neighbour_chunk;
  if((neighbour_chunk = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3(-1,  0,  0))))) neighbour_chunk->mesh_dirty = true;
  if((neighbour_chunk = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 1,  0,  0))))) neighbour_chunk->mesh_dirty = true;
  if((neighbour_chunk = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0, -1,  0))))) neighbour_chunk->mesh_dirty = true;
  if((neighbour_chunk = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  1,  0))))) neighbour_chunk->mesh_dirty = true;
  if((neighbour_chunk = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0, -1))))) neighbour_chunk->mesh_dirty = true;
  if((neighbour_chunk = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0,  1))))) neighbour_chunk->mesh_dirty = true;
}

void world_update(struct world *world, struct window *window, float dt)
{
  world_update_player_control(world, window, dt);
}

