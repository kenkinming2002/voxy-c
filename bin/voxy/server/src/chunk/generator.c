#include "generator.h"

#include "chunk.h"

#include <voxy/server/context.h>

#include <libcore/utils.h>

#include <string.h>

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX voxy_chunk_generator_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk_generator_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t voxy_chunk_generator_wrapper_key(struct voxy_chunk_generator_wrapper *wrapper) { return wrapper->position; }
size_t voxy_chunk_generator_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_chunk_generator_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_chunk_generator_wrapper_dispose(struct voxy_chunk_generator_wrapper *wrapper)
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
  struct voxy_chunk_generator_wrapper *wrapper = container_of(job, struct voxy_chunk_generator_wrapper, job);

  struct voxy_chunk *chunk = voxy_chunk_create();
  chunk->position = wrapper->position;
  chunk->disk_dirty = true;
  chunk->network_dirty = true;

  wrapper->chunk_generator->generate_chunk(wrapper->position, chunk, wrapper->chunk_generator->seed, wrapper->context);

  atomic_store_explicit(&wrapper->chunk, chunk, memory_order_release);
}

static void job_destroy(struct thread_pool_job *job)
{
  // Nothing to do as our lifetime is managed by the hash table and not the job
  // queue.
  (void)job;
}


void voxy_chunk_generator_init(struct voxy_chunk_generator *chunk_generator, seed_t seed)
{
  chunk_generator->seed = seed;
  chunk_generator->generate_chunk = NULL;
  voxy_chunk_generator_wrapper_hash_table_init(&chunk_generator->wrappers);
}

void voxy_chunk_generator_fini(struct voxy_chunk_generator *chunk_generator)
{
  (void)chunk_generator;

  // FIXME: We need to shutdown the global thread pool before we can actually
  //        dispose of all wrappers. Otherwise, we have a use-after-free.
  // voxy_chunk_generator_wrapper_hash_table_dispose(&chunk_generator->wrappers);
}

void voxy_chunk_generator_set_generate_chunk(struct voxy_chunk_generator *chunk_generator, voxy_generate_chunk_t generate_chunk)
{
  chunk_generator->generate_chunk = generate_chunk;
}

struct chunk_future voxy_chunk_generator_generate(struct voxy_chunk_generator *chunk_generator, ivec3_t position, const struct voxy_context *context)
{
  struct voxy_chunk_generator_wrapper *wrapper = voxy_chunk_generator_wrapper_hash_table_lookup(&chunk_generator->wrappers, position);
  if(!wrapper)
  {
    wrapper = malloc(sizeof *wrapper);
    wrapper->position = position;
    wrapper->chunk_generator = chunk_generator;
    wrapper->context = memdup(context, sizeof *context);
    wrapper->job.invoke = job_invoke;
    wrapper->job.destroy = job_destroy;
    atomic_init(&wrapper->chunk, NULL);
    voxy_chunk_generator_wrapper_hash_table_insert(&chunk_generator->wrappers, wrapper);
    thread_pool_enqueue(&wrapper->job);
  }

  struct voxy_chunk *chunk;
  if((chunk = atomic_load_explicit(&wrapper->chunk, memory_order_consume)))
  {
    voxy_chunk_generator_wrapper_hash_table_remove(&chunk_generator->wrappers, position);
    voxy_chunk_generator_wrapper_dispose(wrapper);
    return chunk_future_ready(chunk);
  }

  return chunk_future_pending;
}

