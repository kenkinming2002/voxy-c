#include <voxy/types/world.h>
#include <voxy/types/chunk.h>
#include <voxy/types/chunk_data.h>

#include <voxy/config.h>

#include <stdlib.h>

struct chunk *world_chunk_lookup(struct world *world, ivec3_t position)
{
  return chunk_hash_table_lookup(&world->chunks, position);
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

static inline void split_position(ivec3_t position, ivec3_t *chunk_position, ivec3_t *block_position)
{
  for(int i=0; i<3; ++i)
  {
    (*block_position).values[i] = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    (*chunk_position).values[i] = (position.values[i] - (*block_position).values[i]) / CHUNK_WIDTH;
  }
}

struct block *world_get_block(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(world, chunk_position);
  if(chunk && chunk->chunk_data)
    return &chunk->chunk_data->blocks[block_position.z][block_position.y][block_position.x];
  else
    return NULL;
}

void world_set_block(struct world *world, ivec3_t position, uint8_t block_id)
{
  struct block *block = world_get_block(world, position);
  if(block)
    block->id = block_id;
}

void world_invalidate_light(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(world, chunk_position);
  if(chunk)
    chunk->light_dirty = true;
}

void world_invalidate_mesh(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(world, chunk_position);
  if(chunk)
    chunk->mesh_dirty = true;
}

void world_destroy_block(struct world *world, ivec3_t position)
{
  world_set_block(world, position, 0);

  world_invalidate_light(world, position);

  world_invalidate_mesh(world, position);
  world_invalidate_mesh(world, ivec3_add(position, ivec3(-1, 0, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3( 1, 0, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0, -1, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0,  1, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0, 0, -1)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0, 0,  1)));
}

void world_place_block(struct world *world, ivec3_t position, uint8_t block_id)
{
  world_set_block(world, position, block_id);

  world_invalidate_light(world, position);

  world_invalidate_mesh(world, position);
  world_invalidate_mesh(world, ivec3_add(position, ivec3(-1, 0, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3( 1, 0, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0, -1, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0,  1, 0)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0, 0, -1)));
  world_invalidate_mesh(world, ivec3_add(position, ivec3(0, 0,  1)));
}

