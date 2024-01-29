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

void world_chunk_insert_unchecked(struct world *world, struct chunk *chunk)
{
  chunk->left   = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3(-1,  0,  0)));
  chunk->right  = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 1,  0,  0)));
  chunk->back   = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0, -1,  0)));
  chunk->front  = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  1,  0)));
  chunk->bottom = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0, -1)));
  chunk->top    = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0,  1)));

  if(chunk->left)   chunk->left->right = chunk;
  if(chunk->right)  chunk->right->left = chunk;
  if(chunk->back)   chunk->back->front = chunk;
  if(chunk->front)  chunk->front->back = chunk;
  if(chunk->bottom) chunk->bottom->top = chunk;
  if(chunk->top)    chunk->top->bottom = chunk;

  chunk_hash_table_insert_unchecked(&world->chunks, chunk);
}

void world_update(struct world *world, struct window *window, float dt)
{
  world_update_player_control(world, window, dt);
}

