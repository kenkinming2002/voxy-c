#include <voxy/main_game/world.h>

#include <voxy/config.h>
#include <voxy/types/chunk_data.h>

struct chunk_hash_table chunks;

struct chunk *world_chunk_lookup(ivec3_t position)
{
  return chunk_hash_table_lookup(&chunks, position);
}

void world_chunk_insert_unchecked(struct chunk *chunk)
{
  chunk->left   = world_chunk_lookup(ivec3_add(chunk->position, ivec3(-1,  0,  0)));
  chunk->right  = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 1,  0,  0)));
  chunk->back   = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0, -1,  0)));
  chunk->front  = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  1,  0)));
  chunk->bottom = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  0, -1)));
  chunk->top    = world_chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  0,  1)));

  if(chunk->left)   chunk->left->right = chunk;
  if(chunk->right)  chunk->right->left = chunk;
  if(chunk->back)   chunk->back->front = chunk;
  if(chunk->front)  chunk->front->back = chunk;
  if(chunk->bottom) chunk->bottom->top = chunk;
  if(chunk->top)    chunk->top->bottom = chunk;

  chunk_hash_table_insert_unchecked(&chunks, chunk);
}

static inline void split_position(ivec3_t position, ivec3_t *chunk_position, ivec3_t *block_position)
{
  for(int i=0; i<3; ++i)
  {
    (*block_position).values[i] = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    (*chunk_position).values[i] = (position.values[i] - (*block_position).values[i]) / CHUNK_WIDTH;
  }
}

struct block *world_get_block(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(chunk_position);
  if(!chunk || !chunk->chunk_data)
    return NULL;

  return &chunk->chunk_data->blocks[block_position.z][block_position.y][block_position.x];
}

void world_set_block(ivec3_t position, uint8_t block_id)
{
  struct block *block = world_get_block(position);
  if(!block)
    return;

  block->id = block_id;

  world_invalidate_light_at(position);
  world_invalidate_mesh_at(position);

  world_invalidate_mesh_at(ivec3_add(position, ivec3(-1, 0, 0)));
  world_invalidate_mesh_at(ivec3_add(position, ivec3( 1, 0, 0)));
  world_invalidate_mesh_at(ivec3_add(position, ivec3(0, -1, 0)));
  world_invalidate_mesh_at(ivec3_add(position, ivec3(0,  1, 0)));
  world_invalidate_mesh_at(ivec3_add(position, ivec3(0, 0, -1)));
  world_invalidate_mesh_at(ivec3_add(position, ivec3(0, 0,  1)));
}

void world_invalidate_light_at(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(chunk_position);
  if(!chunk)
    return;

  chunk->light_dirty = true;
}

void world_invalidate_mesh_at(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_chunk_lookup(chunk_position);
  if(!chunk)
    return;

  chunk->mesh_dirty = true;
}

