#include "generator.h"

#include "group.h"

#include <voxy/server/context.h>

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

void voxy_block_generator_init(struct voxy_block_generator *block_generator, const char *world_directory)
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

  block_generator->seed = 0b0101110101011010101110101101010101011010111010100011010100101010;
  seed_combine(&block_generator->seed, seed_data, seed_length);
  seed_combine(&block_generator->seed, seed_data, seed_length);

  LOG_INFO("Seed for block_group generator(string): %.*s", (int)seed_length, seed_data);
  LOG_INFO("Seed for block_group generator(integer): %zu", block_generator->seed);

  block_generator->generate_block = NULL;
  free(seed_data);

  block_generator->wrappers = NULL;
}

void voxy_block_generator_fini(struct voxy_block_generator *block_generator)
{
  (void)block_generator;

  // FIXME: We need to shutdown the global thread pool before we can actually
  //        dispose of all wrappers. Otherwise, we have a use-after-free.
  // voxy_block_generator_wrapper_hash_table_dispose(&block_generator->wrappers);
}

void voxy_block_generator_set_generate_block(struct voxy_block_generator *block_generator, voxy_generate_block_t generate_block)
{
  block_generator->generate_block = generate_block;
}

static void *memdup(const void *data, size_t length)
{
  void *result = malloc(length);
  memcpy(result, data, length);
  return result;
}

struct block_generator_job
{
  struct thread_pool_job job;

  struct voxy_block_generator *block_generator;
  struct voxy_context context;

  ivec3_t position;
  struct voxy_block_group * _Atomic block_group;
};

static void job_invoke(struct thread_pool_job *job)
{
  struct block_generator_job *real_job = container_of(job, struct block_generator_job, job);

  struct voxy_block_group *block_group = voxy_block_group_create();
  block_group->position = real_job->position;
  block_group->disk_dirty = true;
  block_group->network_dirty = true;

  real_job->block_generator->generate_block(real_job->position, block_group, real_job->block_generator->seed, &real_job->context);

  atomic_store_explicit(&real_job->block_group, block_group, memory_order_release);
}

static void job_destroy(struct thread_pool_job *job)
{
  // Nothing to do as our lifetime is managed by the hash table and not the job
  // queue.
  (void)job;
}

static struct block_generator_job *job_create(struct voxy_block_generator *block_generator, ivec3_t position, const struct voxy_context *context)
{
  struct block_generator_job *job = malloc(sizeof *job);

  job->job.invoke = job_invoke;
  job->job.destroy = job_destroy;

  job->block_generator = block_generator;
  memcpy(&job->context, context, sizeof *context);

  job->position = position;
  atomic_init(&job->block_group, NULL);

  return job;
}



struct block_group_future voxy_block_generator_generate(struct voxy_block_generator *block_generator, ivec3_t position, const struct voxy_context *context)
{
  profile_scope;

  struct block_generator_job *job;

  ptrdiff_t i = hmgeti(block_generator->wrappers, position);
  if(i == -1)
  {
    job = job_create(block_generator, position, context);
    thread_pool_enqueue(&job->job);
    hmput(block_generator->wrappers, position, job);
  }
  else
    job = block_generator->wrappers[i].value;

  struct voxy_block_group *block_group;
  if((block_group = atomic_load_explicit(&job->block_group, memory_order_consume)))
  {
    free(job);
    hmdel(block_generator->wrappers, position);
    return block_group_future_ready(block_group);
  }

  return block_group_future_pending;
}

