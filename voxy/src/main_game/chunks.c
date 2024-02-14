#include <voxy/main_game/chunks.h>

#include <voxy/config.h>
#include <voxy/types/chunk_data.h>

struct chunk_hash_table chunks;

struct chunk *chunk_lookup(ivec3_t position)
{
  return chunk_hash_table_lookup(&chunks, position);
}

void chunk_insert_unchecked(struct chunk *chunk)
{
  chunk->left   = chunk_lookup(ivec3_add(chunk->position, ivec3(-1,  0,  0)));
  chunk->right  = chunk_lookup(ivec3_add(chunk->position, ivec3( 1,  0,  0)));
  chunk->back   = chunk_lookup(ivec3_add(chunk->position, ivec3( 0, -1,  0)));
  chunk->front  = chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  1,  0)));
  chunk->bottom = chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  0, -1)));
  chunk->top    = chunk_lookup(ivec3_add(chunk->position, ivec3( 0,  0,  1)));

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

struct block *block_get(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = chunk_lookup(chunk_position);
  if(!chunk || !chunk->chunk_data)
    return NULL;

  return &chunk->chunk_data->blocks[block_position.z][block_position.y][block_position.x];
}

void block_set(ivec3_t position, uint8_t block_id)
{
  struct block *block = block_get(position);
  if(!block)
    return;

  block->id = block_id;

  invalidate_light_at(position);
  invalidate_mesh_at(position);

  invalidate_mesh_at(ivec3_add(position, ivec3(-1, 0, 0)));
  invalidate_mesh_at(ivec3_add(position, ivec3( 1, 0, 0)));
  invalidate_mesh_at(ivec3_add(position, ivec3(0, -1, 0)));
  invalidate_mesh_at(ivec3_add(position, ivec3(0,  1, 0)));
  invalidate_mesh_at(ivec3_add(position, ivec3(0, 0, -1)));
  invalidate_mesh_at(ivec3_add(position, ivec3(0, 0,  1)));
}

void invalidate_light_at(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = chunk_lookup(chunk_position);
  if(!chunk)
    return;

  chunk->light_dirty = true;
}

void invalidate_mesh_at(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = chunk_lookup(chunk_position);
  if(!chunk)
    return;

  chunk->mesh_dirty = true;
}

