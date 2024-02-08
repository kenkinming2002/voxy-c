#include "world_generator.h"

#include <voxy/math/noise.h>

#include <types/world.h>
#include <types/chunk_data.h>

#include "utils.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk_data_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct chunk_data_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <sys/sysinfo.h>
#include <math.h>
#include <stdio.h>

ivec3_t chunk_data_wrapper_key(struct chunk_data_wrapper *chunk_data_wrapper)
{
  return chunk_data_wrapper->position;
}

size_t chunk_data_wrapper_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int chunk_data_wrapper_compare(ivec3_t position1, ivec3_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_data_wrapper_dispose(struct chunk_data_wrapper *chunk_data_wrapper)
{
  free(chunk_data_wrapper->chunk_data);
  free(chunk_data_wrapper);
}

void world_generator_init(struct world_generator *world_generator)
{
  thread_pool_init(&world_generator->thread_pool);
  chunk_data_wrapper_hash_table_init(&world_generator->chunk_data_wrappers);
}

void world_generator_fini(struct world_generator *world_generator)
{
  thread_pool_fini(&world_generator->thread_pool);
  chunk_data_wrapper_hash_table_dispose(&world_generator->chunk_data_wrappers);
}

struct chunk_data_generate_job
{
  struct thread_pool_job base;

  seed_t                     seed;
  struct chunk_data_wrapper *chunk_data_wrapper;
  struct resource_pack      *resource_pack;
};

static void world_generator_generate_chunk_data_job_invoke(struct thread_pool_job *_job)
{
  struct chunk_data_generate_job *job = container_of(_job, struct chunk_data_generate_job, base);
  struct chunk_data *chunk_data = malloc(sizeof *chunk_data);

  uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  job->resource_pack->generate_blocks(job->seed, job->chunk_data_wrapper->position, blocks);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        chunk_data->blocks[z][y][x].id          = blocks[z][y][x];
        chunk_data->blocks[z][y][x].ether       = false;
        chunk_data->blocks[z][y][x].light_level = 0;
      }

  atomic_store_explicit(&job->chunk_data_wrapper->chunk_data, chunk_data, memory_order_release);
}

static void world_generator_generate_chunk_data_job_destroy(struct thread_pool_job *_job)
{
  struct chunk_data_generate_job *job = container_of(_job, struct chunk_data_generate_job, base);
  free(job);
}

struct chunk_data *world_generator_generate_chunk_data(struct world_generator *world_generator, struct world *world, ivec3_t position, struct resource_pack *resource_pack)
{
  struct chunk_data_wrapper *chunk_data_wrapper;
  if((chunk_data_wrapper = chunk_data_wrapper_hash_table_lookup(&world_generator->chunk_data_wrappers, position)))
    return atomic_exchange_explicit(&chunk_data_wrapper->chunk_data, NULL, memory_order_consume);

  chunk_data_wrapper = malloc(sizeof *chunk_data_wrapper);
  chunk_data_wrapper->position = position;
  chunk_data_wrapper->chunk_data = NULL;
  chunk_data_wrapper_hash_table_insert_unchecked(&world_generator->chunk_data_wrappers, chunk_data_wrapper);

  struct chunk_data_generate_job *job = malloc(sizeof *job);
  job->base.invoke        = world_generator_generate_chunk_data_job_invoke;
  job->base.destroy       = world_generator_generate_chunk_data_job_destroy;
  job->seed               = world->seed;
  job->chunk_data_wrapper = chunk_data_wrapper;
  job->resource_pack      = resource_pack;
  thread_pool_enqueue(&world_generator->thread_pool, &job->base);
  return NULL;
}

