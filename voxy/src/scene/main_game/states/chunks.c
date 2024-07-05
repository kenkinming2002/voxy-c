#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/invalidate.h>

#include <stdlib.h>

struct chunk_hash_table world_chunks;

struct chunk *world_get_chunk(ivec3_t position)
{
  struct chunk *chunk = chunk_hash_table_lookup(&world_chunks, position);
  if(!chunk)
  {
    chunk = calloc(sizeof *chunk, 1);
    chunk->position = position;
    chunk_hash_table_insert_unchecked(&world_chunks, chunk);

    chunk->left   = chunk_hash_table_lookup(&world_chunks, ivec3_add(chunk->position, ivec3(-1,  0,  0)));
    chunk->right  = chunk_hash_table_lookup(&world_chunks, ivec3_add(chunk->position, ivec3( 1,  0,  0)));
    chunk->back   = chunk_hash_table_lookup(&world_chunks, ivec3_add(chunk->position, ivec3( 0, -1,  0)));
    chunk->front  = chunk_hash_table_lookup(&world_chunks, ivec3_add(chunk->position, ivec3( 0,  1,  0)));
    chunk->bottom = chunk_hash_table_lookup(&world_chunks, ivec3_add(chunk->position, ivec3( 0,  0, -1)));
    chunk->top    = chunk_hash_table_lookup(&world_chunks, ivec3_add(chunk->position, ivec3( 0,  0,  1)));

    if(chunk->left)   chunk->left->right = chunk;
    if(chunk->right)  chunk->right->left = chunk;
    if(chunk->back)   chunk->back->front = chunk;
    if(chunk->front)  chunk->front->back = chunk;
    if(chunk->bottom) chunk->bottom->top = chunk;
    if(chunk->top)    chunk->top->bottom = chunk;
  }
  return chunk;
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

  struct chunk *chunk = world_get_chunk(chunk_position);
  return chunk_get_block(chunk, block_position);
}

static void world_invalidate_block_impl(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_get_chunk(chunk_position);
  world_invalidate_chunk_light(chunk);
  world_invalidate_chunk_mesh(chunk);
}

void world_invalidate_block(ivec3_t position)
{
  world_invalidate_block_impl(position);
  world_invalidate_block_impl(ivec3_add(position, ivec3(-1, 0, 0)));
  world_invalidate_block_impl(ivec3_add(position, ivec3( 1, 0, 0)));
  world_invalidate_block_impl(ivec3_add(position, ivec3(0, -1, 0)));
  world_invalidate_block_impl(ivec3_add(position, ivec3(0,  1, 0)));
  world_invalidate_block_impl(ivec3_add(position, ivec3(0, 0, -1)));
  world_invalidate_block_impl(ivec3_add(position, ivec3(0, 0,  1)));
}

struct entity *world_add_entity(struct entity entity)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(fvec3_as_ivec3_round(entity.position), &chunk_position, &block_position);

  struct chunk *chunk = world_get_chunk(chunk_position);
  return chunk_add_entity(chunk, entity);
}

