#include "world_generator.h"

#include <voxy/math/noise.h>

#include "world.h"
#include "utils.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX section_info
#define SC_HASH_TABLE_NODE_TYPE struct section_info
#define SC_HASH_TABLE_KEY_TYPE ivec2_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk_data_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct chunk_data_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <sys/sysinfo.h>
#include <math.h>
#include <stdio.h>

////////////////////
/// Section Info ///
////////////////////
ivec2_t section_info_key(struct section_info *section_info)
{
  return section_info->position;
}

size_t section_info_hash(ivec2_t position)
{
  return ivec2_hash(position);
}

int section_info_compare(ivec2_t position1, ivec2_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  return 0;
}

void section_info_dispose(struct section_info *section_info)
{
  free(section_info);
}

//////////////////
/// Chunk Info ///
//////////////////
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
  free(chunk_data_wrapper);
}

/////////////////
/// Init/Fini ///
/////////////////
void world_generator_init(struct world_generator *world_generator, seed_t seed)
{
  world_generator->seed = seed;
  thread_pool_init(&world_generator->thread_pool);
  section_info_hash_table_init(&world_generator->section_infos);
  chunk_data_wrapper_hash_table_init(&world_generator->chunk_data_wrappers);
}

void world_generator_fini(struct world_generator *world_generator)
{
  thread_pool_fini(&world_generator->thread_pool);
  section_info_hash_table_dispose(&world_generator->section_infos);
  chunk_data_wrapper_hash_table_dispose(&world_generator->chunk_data_wrappers);
}

////////////
/// Jobs ///
////////////
struct section_info_generate_job
{
  struct thread_pool_job base;

  seed_t                seed;
  struct section_info  *section_info;
  struct resource_pack *resource_pack;
};

struct chunk_data_generate_job
{
  struct thread_pool_job base;

  seed_t                     seed;
  struct section_info       *section_info;
  struct chunk_data_wrapper *chunk_data_wrapper;
  struct resource_pack      *resource_pack;
};

static void world_generator_generate_section_info_job_invoke(struct thread_pool_job *_job)
{
  struct section_info_generate_job *job = container_of(_job, struct section_info_generate_job, base);
  job->resource_pack->generate_heights(job->seed, job->section_info->position, job->section_info->heights);
  atomic_store_explicit(&job->section_info->done, true, memory_order_release);
}

static void world_generator_generate_section_info_job_destroy(struct thread_pool_job *_job)
{
  struct section_info_generate_job *job = container_of(_job, struct section_info_generate_job, base);
  free(job);
}

static void world_generator_generate_chunk_data_job_invoke(struct thread_pool_job *_job)
{
  struct chunk_data_generate_job *job = container_of(_job, struct chunk_data_generate_job, base);
  struct chunk_data *chunk_data = malloc(sizeof *chunk_data);

  uint8_t tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  job->resource_pack->generate_tiles(job->seed, job->chunk_data_wrapper->position, job->section_info->heights, tiles);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        chunk_data->tiles[z][y][x].id          = tiles[z][y][x];
        chunk_data->tiles[z][y][x].ether       = false;
        chunk_data->tiles[z][y][x].light_level = 0;
      }

  atomic_store_explicit(&job->chunk_data_wrapper->chunk_data, chunk_data, memory_order_release);
}

static void world_generator_generate_chunk_data_job_destroy(struct thread_pool_job *_job)
{
  struct chunk_data_generate_job *job = container_of(_job, struct chunk_data_generate_job, base);
  free(job);
}

/////////////////
/// Functions ///
/////////////////
struct section_info *world_generator_generate_section_info(struct world_generator *world_generator, ivec2_t position, struct resource_pack *resource_pack)
{
  struct section_info *section_info;
  if((section_info = section_info_hash_table_lookup(&world_generator->section_infos, position)))
    return atomic_load_explicit(&section_info->done, memory_order_acquire) ? section_info : NULL;

  section_info = malloc(sizeof *section_info);
  section_info->position = position;
  section_info->done     = false;
  section_info_hash_table_insert_unchecked(&world_generator->section_infos, section_info);

  struct section_info_generate_job *job = malloc(sizeof *job);
  job->base.invoke   = world_generator_generate_section_info_job_invoke;
  job->base.destroy  = world_generator_generate_section_info_job_destroy;
  job->seed          = world_generator->seed;
  job->section_info  = section_info;
  job->resource_pack = resource_pack;
  thread_pool_enqueue(&world_generator->thread_pool, &job->base);
  return NULL;
}

struct chunk_data *world_generator_generate_chunk_data(struct world_generator *world_generator, ivec3_t position, struct resource_pack *resource_pack)
{
  struct section_info *section_info = world_generator_generate_section_info(world_generator, ivec2(position.x, position.y), resource_pack);
  if(!section_info)
    return NULL;

  struct chunk_data_wrapper *chunk_data_wrapper;
  if((chunk_data_wrapper = chunk_data_wrapper_hash_table_lookup(&world_generator->chunk_data_wrappers, position)))
    return atomic_load_explicit(&chunk_data_wrapper->chunk_data, memory_order_consume) ? chunk_data_wrapper->chunk_data : NULL;

  chunk_data_wrapper = malloc(sizeof *chunk_data_wrapper);
  chunk_data_wrapper->position = position;
  chunk_data_wrapper->chunk_data = NULL;
  chunk_data_wrapper_hash_table_insert_unchecked(&world_generator->chunk_data_wrappers, chunk_data_wrapper);

  struct chunk_data_generate_job *job = malloc(sizeof *job);
  job->base.invoke        = world_generator_generate_chunk_data_job_invoke;
  job->base.destroy       = world_generator_generate_chunk_data_job_destroy;
  job->seed               = world_generator->seed;
  job->section_info       = section_info;
  job->chunk_data_wrapper = chunk_data_wrapper;
  job->resource_pack      = resource_pack;
  thread_pool_enqueue(&world_generator->thread_pool, &job->base);
  return NULL;
}

////////////
/// Misc ///
////////////
float world_generator_get_height(struct world_generator *world_generator, ivec2_t position, struct resource_pack *resource_pack)
{
  ivec2_t chunk_position;
  ivec2_t chunk_offset;

  chunk_offset.x = (position.x % CHUNK_WIDTH + CHUNK_WIDTH) % CHUNK_WIDTH;
  chunk_offset.y = (position.y % CHUNK_WIDTH + CHUNK_WIDTH) % CHUNK_WIDTH;

  chunk_position.x = (position.x - chunk_offset.x) / CHUNK_WIDTH;
  chunk_position.y = (position.y - chunk_offset.y) / CHUNK_WIDTH;

  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
  resource_pack->generate_heights(world_generator->seed, chunk_position, heights);
  return heights[chunk_offset.y][chunk_offset.x];
}
