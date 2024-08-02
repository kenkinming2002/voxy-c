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

ivec3_t get_chunk_position_i(ivec3_t position)
{
  return ivec3_div_scalar(ivec3_sub(position, global_position_to_local_position_i(position)), CHUNK_WIDTH);
}

ivec3_t get_chunk_position_f(fvec3_t position)
{
  return get_chunk_position_i(fvec3_as_ivec3_round(position));
}

ivec3_t local_position_to_global_position_i(ivec3_t position, ivec3_t chunk_position)
{
  return ivec3_add(ivec3_mul_scalar(chunk_position, CHUNK_WIDTH), position);
}

fvec3_t local_position_to_global_position_f(fvec3_t position, ivec3_t chunk_position)
{
  return fvec3_add(ivec3_as_fvec3(ivec3_mul_scalar(chunk_position, CHUNK_WIDTH)), position);
}

ivec3_t global_position_to_local_position_i(ivec3_t position)
{
  for(int i=0; i<3; ++i)
  {
    position.values[i] %= CHUNK_WIDTH;
    position.values[i] += CHUNK_WIDTH;
    position.values[i] %= CHUNK_WIDTH;
  }
  return position;
}

fvec3_t global_position_to_local_position_f(fvec3_t position)
{
  for(int i=0; i<3; ++i)
  {
    position.values[i] += 0.5f;
    position.values[i] = fmodf(position.values[i], CHUNK_WIDTH);
    position.values[i] -= 0.5f;
  }
  return position;
}

void world_invalidate_mesh_at(ivec3_t position)
{
  struct chunk *chunk = world_get_chunk(get_chunk_position_i(position));
  if(chunk && chunk->data)
    chunk_invalidate_mesh_at(chunk, global_position_to_local_position_i(position));
}

block_id_t world_get_block_id(ivec3_t position)
{
  struct chunk *chunk = world_get_chunk(get_chunk_position_i(position));
  if(chunk && chunk->data)
    return chunk_get_block_id(chunk, global_position_to_local_position_i(position));
  else
    return BLOCK_NONE;
}

unsigned world_get_block_light_level(ivec3_t position)
{
  struct chunk *chunk = world_get_chunk(get_chunk_position_i(position));
  if(chunk && chunk->data)
    return chunk_get_block_light_level(chunk, global_position_to_local_position_i(position));
  else
    return -1;
}

void world_set_block(ivec3_t position, block_id_t id, struct entity *entity)
{
  struct chunk *chunk = world_get_chunk(get_chunk_position_i(position));
  if(chunk && chunk->data)
  {
    {
      const block_id_t block_id = chunk_get_block_id(chunk, global_position_to_local_position_i(position));
      const struct block_info *block_info = query_block_info(block_id);
      if(block_info->on_destroy)
        block_info->on_destroy(entity, chunk, global_position_to_local_position_i(position));
    }
    chunk_set_block(chunk, global_position_to_local_position_i(position), id);
    {
      const block_id_t block_id = chunk_get_block_id(chunk, global_position_to_local_position_i(position));
      const struct block_info *block_info = query_block_info(block_id);
      if(block_info->on_create)
        block_info->on_create(entity, chunk, global_position_to_local_position_i(position));
    }
  }
}

bool world_add_entity_raw(struct entity entity)
{
  struct chunk *chunk = world_get_chunk(get_chunk_position_f(entity.position));
  if(chunk && chunk->data)
  {
    chunk_add_entity_raw(chunk, entity);
    return true;
  }
  else
    return false;
}

bool world_add_entity(struct entity entity)
{
  struct chunk *chunk = world_get_chunk(get_chunk_position_f(entity.position));
  if(chunk && chunk->data)
  {
    chunk_add_entity(chunk, entity);
    return true;
  }
  else
    return false;
}

