#include <types/world.h>
#include <types/chunk.h>
#include <types/chunk_data.h>

#include <voxy/config.h>

#include <stdlib.h>

void world_init(struct world *world, seed_t seed)
{
  world->seed = seed;

  chunk_hash_table_init(&world->chunks);

  world->player.spawned = false;
}

void world_fini(struct world *world)
{
  chunk_hash_table_dispose(&world->chunks);
}

struct block *world_get_block(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  for(int i=0; i<3; ++i)
  {
    block_position.values[i]  = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    chunk_position.values[i] = (position.values[i] - block_position.values[i]) / CHUNK_WIDTH;
  }

  struct chunk *chunk = chunk_hash_table_lookup(&world->chunks, chunk_position);
  if(!chunk || !chunk->chunk_data)
    return NULL;

  return &chunk->chunk_data->blocks[block_position.z][block_position.y][block_position.x];
}

void world_invalidate_block(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t block_position;
  for(int i=0; i<3; ++i)
  {
    block_position.values[i]  = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    chunk_position.values[i] = (position.values[i] - block_position.values[i]) / CHUNK_WIDTH;
  }

  struct chunk *chunk = chunk_hash_table_lookup(&world->chunks, chunk_position);
  if(chunk)
  {
    chunk->light_dirty = true;
    chunk->mesh_dirty = true;
  }
}

void world_block_set_id(struct world *world, ivec3_t position, uint8_t id)
{
  struct block *block = world_get_block(world, position);
  if(block)
  {
    block->id = id;

    world_invalidate_block(world, position);
    world_invalidate_block(world, ivec3_add(position, ivec3(-1, 0, 0)));
    world_invalidate_block(world, ivec3_add(position, ivec3( 1, 0, 0)));
    world_invalidate_block(world, ivec3_add(position, ivec3(0, -1, 0)));
    world_invalidate_block(world, ivec3_add(position, ivec3(0,  1, 0)));
    world_invalidate_block(world, ivec3_add(position, ivec3(0, 0, -1)));
    world_invalidate_block(world, ivec3_add(position, ivec3(0, 0,  1)));
  }
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

