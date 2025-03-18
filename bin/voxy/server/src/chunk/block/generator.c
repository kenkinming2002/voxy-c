#include "generator.h"

#include "group.h"

#include <voxy/server/context.h>

#include <libcore/format.h>
#include <libcore/utils.h>
#include <libcore/profile.h>
#include <libcore/fs.h>
#include <libcore/log.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX voxy_block_generator_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_block_generator_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t voxy_block_generator_wrapper_key(struct voxy_block_generator_wrapper *wrapper) { return wrapper->position; }
size_t voxy_block_generator_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_block_generator_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_block_generator_wrapper_dispose(struct voxy_block_generator_wrapper *wrapper)
{
  free(wrapper->context);
  free(wrapper);
}

static void *memdup(const void *data, size_t length)
{
  void *result = malloc(length);
  memcpy(result, data, length);
  return result;
}

static void job_invoke(struct thread_pool_job *job)
{
  struct voxy_block_generator_wrapper *wrapper = container_of(job, struct voxy_block_generator_wrapper, job);

  struct voxy_block_group *block_group = voxy_block_group_create();
  block_group->position = wrapper->position;
  block_group->disk_dirty = true;
  block_group->network_dirty = true;

  wrapper->block_generator->generate_block(wrapper->position, block_group, wrapper->block_generator->seed, wrapper->context);

  atomic_store_explicit(&wrapper->block_group, block_group, memory_order_release);
}

static void job_destroy(struct thread_pool_job *job)
{
  // Nothing to do as our lifetime is managed by the hash table and not the job
  // queue.
  (void)job;
}


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

  free(seed_data);

  voxy_block_generator_wrapper_hash_table_init(&block_generator->wrappers);

  block_generator->generate_block = NULL;
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

struct block_group_future voxy_block_generator_generate(struct voxy_block_generator *block_generator, ivec3_t position, const struct voxy_context *context)
{
  profile_scope;

  struct voxy_block_generator_wrapper *wrapper = voxy_block_generator_wrapper_hash_table_lookup(&block_generator->wrappers, position);
  if(!wrapper)
  {
    wrapper = malloc(sizeof *wrapper);
    wrapper->position = position;
    wrapper->block_generator = block_generator;
    wrapper->context = memdup(context, sizeof *context);
    wrapper->job.invoke = job_invoke;
    wrapper->job.destroy = job_destroy;
    atomic_init(&wrapper->block_group, NULL);
    voxy_block_generator_wrapper_hash_table_insert(&block_generator->wrappers, wrapper);
    thread_pool_enqueue(&wrapper->job);
  }

  struct voxy_block_group *block_group;
  if((block_group = atomic_load_explicit(&wrapper->block_group, memory_order_consume)))
  {
    voxy_block_generator_wrapper_hash_table_remove(&block_generator->wrappers, position);
    voxy_block_generator_wrapper_dispose(wrapper);
    return block_group_future_ready(block_group);
  }

  return block_group_future_pending;
}

