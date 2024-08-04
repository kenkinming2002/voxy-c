#include <voxy/scene/main_game/update/chunk_generate.h>

#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/types/chunk_hash_table.h>

#include <voxy/core/thread_pool.h>
#include <voxy/core/log.h>
#include <voxy/core/time.h>

#include <voxy/utils.h>

#include <stdatomic.h>

static struct chunk *generate_chunk_impl(ivec3_t position)
{
  struct chunk *chunk = malloc(sizeof *chunk);
  chunk->position = position;
  chunk->dirty = true;
  chunk->last_save_time = get_time();

  block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  generate_chunk_blocks(world_seed_get(), chunk->position, blocks);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
        chunk_set_block_raw(chunk, ivec3(x, y, z), blocks[z][y][x]);

  DYNAMIC_ARRAY_INIT(chunk->entities);
  DYNAMIC_ARRAY_INIT(chunk->new_entities);
  return chunk;
}

struct wrapper
{
  ivec3_t         position;
  struct wrapper *next;
  size_t          hash;

  struct thread_pool_job job;
  struct chunk * _Atomic chunk;
};

static void job_invoke(struct thread_pool_job *job)
{
  struct wrapper *wrapper = container_of(job, struct wrapper, job);
  atomic_store_explicit(&wrapper->chunk, generate_chunk_impl(wrapper->position), memory_order_release);
}

static void job_destroy(struct thread_pool_job *job)
{
  // Nothing to do as our lifetime is managed by the hash table and not the job
  // queue.
  (void)job;
}

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX wrapper
#define SC_HASH_TABLE_NODE_TYPE struct wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t wrapper_key(struct wrapper *wrapper) { return wrapper->position; }
size_t wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void wrapper_dispose(struct wrapper *wrapper) { free(wrapper); }

static struct wrapper_hash_table wrappers;

bool is_chunk_generating(ivec3_t position)
{
  return wrapper_hash_table_lookup(&wrappers, position);
}

struct chunk *generate_chunk(ivec3_t position)
{
  struct wrapper *wrapper = wrapper_hash_table_lookup(&wrappers, position);
  if(!wrapper)
  {
    wrapper = malloc(sizeof *wrapper);
    wrapper->position = position;
    wrapper->job.invoke = job_invoke;
    wrapper->job.destroy = job_destroy;
    atomic_init(&wrapper->chunk, NULL);
    wrapper_hash_table_insert(&wrappers, wrapper);
    thread_pool_enqueue(&wrapper->job);
  }

  struct chunk *chunk;
  if((chunk = atomic_load_explicit(&wrapper->chunk, memory_order_consume)))
  {
    wrapper_hash_table_remove(&wrappers, position);
    free(wrapper);
  }

  return chunk;
}

