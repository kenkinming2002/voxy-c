#include "generator.h"

#include "group.h"

#include <libcore/format.h>
#include <libcore/fs.h>
#include <libcore/log.h>
#include <libcore/profile.h>
#include <libcore/thread_pool.h>
#include <libcore/utils.h>

#include <stb_ds.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>

struct block_generator_wrappers
{
  ivec3_t key;
  struct block_generator_job *value;
};

static seed_t seed;
static voxy_generate_block_t generate_block;

static struct block_generator_wrappers *wrappers;

void voxy_block_generator_init(const char *world_directory)
{
  const char *path = tformat("%s/chunks/seed", world_directory);

  char *seed_data;
  size_t seed_length;
  if(read_file_all(path, &seed_data, &seed_length) != 0)
  {
    seed_length = 32;
    seed_data = malloc(seed_length);

    srand(time(NULL));
    for(unsigned i=0; i<seed_length; ++i)
      while(!isalnum(seed_data[i] = rand()));

    LOG_INFO("Failed to find seed for block_group generator. Generated a new seed %.*s", (int)seed_length, seed_data);

    if(mkdir_recursive(tformat("%s/chunks", world_directory)) != 0)
    {
      LOG_INFO("Failed to create directory for block_group generator seed file: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if(write_file_all(path, seed_data, seed_length) != 0)
    {
      LOG_INFO("Failed to write block_group generator seed file: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  seed = 0b0101110101011010101110101101010101011010111010100011010100101010;
  seed_combine(&seed, seed_data, seed_length);
  seed_combine(&seed, seed_data, seed_length);

  LOG_INFO("Seed for block_group generator(string): %.*s", (int)seed_length, seed_data);
  LOG_INFO("Seed for block_group generator(integer): %zu", seed);

  generate_block = NULL;
  free(seed_data);

  wrappers = NULL;
}

void voxy_set_generate_block(voxy_generate_block_t _generate_block)
{
  generate_block = _generate_block;
}

struct block_generator_job
{
  struct thread_pool_job job;

  ivec3_t position;
  struct voxy_block_group * _Atomic block_group;
};

static void job_invoke(struct thread_pool_job *job)
{
  struct block_generator_job *real_job = container_of(job, struct block_generator_job, job);

  voxy_block_id_t blocks[VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH];
  generate_block(seed, real_job->position, blocks);

  struct voxy_block_group *block_group = voxy_block_group_create();

  for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        voxy_block_group_set_id(block_group, ivec3(x, y, z), blocks[z][y][x]);
        voxy_block_group_set_light_level(block_group, ivec3(x, y, z), voxy_query_block(blocks[z][y][x]).light_level);
      }

  block_group->disk_dirty = true;
  block_group->network_dirty = true;

  atomic_store_explicit(&real_job->block_group, block_group, memory_order_release);
}

static void job_destroy(struct thread_pool_job *job)
{
  // Nothing to do as our lifetime is managed by the hash table and not the job
  // queue.
  (void)job;
}

static struct block_generator_job *job_create(ivec3_t position)
{
  struct block_generator_job *job = malloc(sizeof *job);

  job->job.invoke = job_invoke;
  job->job.destroy = job_destroy;

  job->position = position;
  atomic_init(&job->block_group, NULL);

  return job;
}

struct block_group_future voxy_block_group_generate(ivec3_t position)
{
  profile_scope;

  struct block_generator_job *job;

  ptrdiff_t i = hmgeti(wrappers, position);
  if(i == -1)
  {
    job = job_create(position);
    thread_pool_enqueue(&job->job);
    hmput(wrappers, position, job);
  }
  else
    job = wrappers[i].value;

  struct voxy_block_group *block_group;
  if((block_group = atomic_load_explicit(&job->block_group, memory_order_consume)))
  {
    free(job);
    hmdel(wrappers, position);
    return block_group_future_ready(block_group);
  }

  return block_group_future_pending;
}

