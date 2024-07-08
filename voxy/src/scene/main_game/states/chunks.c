#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/update/light.h>

#include <voxy/core/log.h>

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

bool world_get_block_ex(ivec3_t position, struct chunk **chunk, struct block **block)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  *chunk = world_get_chunk(chunk_position);
  *block = chunk_get_block(*chunk, block_position);
  return *block;
}

/// Set a block at position. Pass id == ID_NONE to destroy the block. This takes
/// of sending invalidation events to all relevant systems, and calling all
/// relevant callbacks.
void world_set_block(ivec3_t position, block_id_t id, struct entity *entity)
{
  struct chunk *chunk;
  struct block *block;

  if(world_get_block_ex(position, &chunk, &block))
  {
    // Opaque => Transparent : destruction
    // Transparent => Opaque : destruction

    // Destroy
    {
      const struct block_info *block_info = query_block_info(block->id);
      if(block_info->on_destroy)
        block_info->on_destroy(entity, chunk, block);
    }

    // Create
    {
      const struct block_info *block_info = query_block_info(id);

      const unsigned old_light_level = block->light_level;
      const unsigned old_ether = block->ether;

      block->id = id;
      block->light_level = block_info->light_level;
      block->ether = block_info->ether;

      if(old_light_level < block->light_level || old_ether < block->ether)
        enqueue_light_create_update(position);

      if(old_light_level >= block->light_level || old_ether >= block->ether)
        enqueue_light_destroy_update(position, old_light_level, old_ether);

      if(block_info->on_create)
        block_info->on_create(entity, chunk, block);
    }

    // Invalidate
    world_invalidate_block(position);
  }
}

static void world_invalidate_block_impl(ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  split_position(position, &chunk_position, &block_position);

  struct chunk *chunk = world_get_chunk(chunk_position);
  if(chunk)
    chunk->mesh_invalidated = true;
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

