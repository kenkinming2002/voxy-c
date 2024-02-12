#include <main_game/chunk_generate.h>
#include <main_game/world.h>
#include <main_game/mod.h>

#include <types/world.h>
#include <types/chunk.h>
#include <types/chunk_data.h>

#include <core/thread_pool.h>

#include <sc/hash_table.h>

#include <utils.h>
#include <config.h>

struct chunk_generate_wrapper
{
  struct chunk_generate_wrapper *next;
  size_t                         hash;
  ivec3_t                        position;

  struct thread_pool_job job;

  struct chunk_data * _Atomic chunk_data;
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
  free(chunk_generate_wrapper->chunk_data);
  free(chunk_generate_wrapper);
}

void chunk_generate_wrapper_invoke(struct thread_pool_job *job)
{
  struct chunk_generate_wrapper *wrapper = container_of(job, struct chunk_generate_wrapper, job);
  struct chunk_data *chunk_data = malloc(sizeof *chunk_data);

  uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  mod_generate_blocks(world.seed, wrapper->position, blocks);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        chunk_data->blocks[z][y][x].id          = blocks[z][y][x];
        chunk_data->blocks[z][y][x].ether       = false;
        chunk_data->blocks[z][y][x].light_level = 0;
      }

  atomic_store_explicit(&wrapper->chunk_data, chunk_data, memory_order_release);
}

void chunk_generate_wrapper_destroy(struct thread_pool_job *job)
{
  (void)job;
}

static struct chunk_generate_wrapper_hash_table chunk_generate_wrappers;

void update_chunk_generate(void)
{
  ivec3_t player_position = fvec3_as_ivec3_floor(fvec3_div_scalar(world.player.base.position, CHUNK_WIDTH));
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        ivec3_t position = ivec3_add(player_position, ivec3(dx, dy, dz));

        struct chunk *chunk = chunk_hash_table_lookup(&world.chunks, position);
        if(chunk)
          continue;

        struct chunk_generate_wrapper *wrapper = chunk_generate_wrapper_hash_table_lookup(&chunk_generate_wrappers, position);
        if(!wrapper)
        {
          struct chunk_generate_wrapper *wrapper = malloc(sizeof *wrapper);
          wrapper->position = ivec3_add(player_position, ivec3(dx, dy, dz));
          wrapper->job.invoke = chunk_generate_wrapper_invoke;
          wrapper->job.destroy = chunk_generate_wrapper_destroy;
          wrapper->chunk_data = NULL;

          chunk_generate_wrapper_hash_table_insert_unchecked(&chunk_generate_wrappers, wrapper);
          thread_pool_enqueue(&wrapper->job);
          continue;
        }

        struct chunk_data *chunk_data = atomic_load_explicit(&wrapper->chunk_data, memory_order_consume);
        if(!chunk_data)
          continue;

        chunk = malloc(sizeof *chunk);
        chunk->position = position;
        chunk->chunk_data = chunk_data;
        chunk->chunk_mesh = NULL;

        world_chunk_insert_unchecked(&world, chunk);

        chunk->mesh_dirty  = true;
        chunk->light_dirty = true;

        if(chunk->left)   chunk->left  ->mesh_dirty = true;
        if(chunk->right)  chunk->right ->mesh_dirty = true;
        if(chunk->back)   chunk->back  ->mesh_dirty = true;
        if(chunk->front)  chunk->front ->mesh_dirty = true;
        if(chunk->bottom) chunk->bottom->mesh_dirty = true;
        if(chunk->top)    chunk->top   ->mesh_dirty = true;
      }
}
