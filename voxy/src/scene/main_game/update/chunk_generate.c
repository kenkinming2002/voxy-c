#include <voxy/scene/main_game/update/chunk_generate.h>

#include <voxy/scene/main_game/config.h>

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>

#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/update/light.h>

#include <voxy/core/thread_pool.h>
#include <voxy/core/log.h>

#include <voxy/utils.h>

#include <sc/hash_table.h>

#include <stdlib.h>
#include <stdio.h>

static ivec3_t *chunk_generate_positions;
static size_t   chunk_generate_position_count;
static size_t   chunk_generate_position_capacity;

void enqueue_chunk_generate(ivec3_t position)
{
  if(chunk_generate_position_count == chunk_generate_position_capacity)
  {
    chunk_generate_position_capacity = chunk_generate_position_capacity != 0 ? 2 * chunk_generate_position_capacity : 1;
    chunk_generate_positions = realloc(chunk_generate_positions, chunk_generate_position_capacity * sizeof *chunk_generate_positions);
  }
  chunk_generate_positions[chunk_generate_position_count++] = position;
}

struct chunk_generate_wrapper
{
  struct chunk_generate_wrapper *next;
  size_t                         hash;
  ivec3_t                        position;

  struct thread_pool_job job;

  atomic_bool done;
  block_id_t  blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk_generate_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct chunk_generate_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION
#undef SC_HASH_TABLE_INTERFACE

ivec3_t chunk_generate_wrapper_key(struct chunk_generate_wrapper *chunk_generate_wrapper)
{
  return chunk_generate_wrapper->position;
}

size_t chunk_generate_wrapper_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int chunk_generate_wrapper_compare(ivec3_t position1, ivec3_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_generate_wrapper_dispose(struct chunk_generate_wrapper *chunk_generate_wrapper)
{
  free(chunk_generate_wrapper);
}

void chunk_generate_wrapper_invoke(struct thread_pool_job *job)
{
  struct chunk_generate_wrapper *wrapper = container_of(job, struct chunk_generate_wrapper, job);
  generate_chunk_blocks(world_seed_get(), wrapper->position, wrapper->blocks);
  atomic_store_explicit(&wrapper->done, true, memory_order_release);
}

void chunk_generate_wrapper_destroy(struct thread_pool_job *job)
{
  (void)job;
}

static struct chunk_generate_wrapper_hash_table chunk_generate_wrappers;
static bool update_generate_chunk_at(ivec3_t position)
{
  struct chunk *chunk = world_get_chunk(position);
  if(chunk->data)
    return false;

  struct chunk_generate_wrapper *wrapper = chunk_generate_wrapper_hash_table_lookup(&chunk_generate_wrappers, position);
  if(!wrapper)
  {
    wrapper = malloc(sizeof *wrapper);
    wrapper->job.invoke  = chunk_generate_wrapper_invoke;
    wrapper->job.destroy = chunk_generate_wrapper_destroy;
    wrapper->position = position;
    wrapper->done = false;

    thread_pool_enqueue(&wrapper->job);
    chunk_generate_wrapper_hash_table_insert(&chunk_generate_wrappers, wrapper);
    return false;
  }

  if(!atomic_load_explicit(&wrapper->done, memory_order_acquire))
    return false;

  chunk->data = malloc(sizeof *chunk->data);

  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        block_id_t block_id = wrapper->blocks[z][y][x];
        const struct block_info *block_info = query_block_info(block_id);

        chunk->data->blocks[z][y][x].id = block_id;
        chunk->data->blocks[z][y][x].ether = block_info->ether;
        chunk->data->blocks[z][y][x].light_level = block_info->light_level;

        if(block_info->ether != 0 || block_info->light_level != 0)
          enqueue_light_create_update_raw(chunk, x, y, z);
      }

  DYNAMIC_ARRAY_INIT(chunk->data->entities);
  DYNAMIC_ARRAY_INIT(chunk->data->new_entities);

  chunk->mesh_invalidated = true;
  if(chunk->left) chunk->left->mesh_invalidated = true;
  if(chunk->right) chunk->right->mesh_invalidated = true;
  if(chunk->back) chunk->back->mesh_invalidated = true;
  if(chunk->front) chunk->front->mesh_invalidated = true;
  if(chunk->bottom) chunk->bottom->mesh_invalidated = true;
  if(chunk->top) chunk->top->mesh_invalidated = true;

  free(chunk_generate_wrapper_hash_table_remove(&chunk_generate_wrappers, position));
  return true;
}

void update_chunk_generate(void)
{
  size_t count = 0;
  for(size_t i=0; i<chunk_generate_position_count; ++i)
    count += update_generate_chunk_at(chunk_generate_positions[i]);
  chunk_generate_position_count = 0;

  if(count != 0)
    LOG_INFO("Chunk Generator: Generarted %zu chunks in background", count);
}
