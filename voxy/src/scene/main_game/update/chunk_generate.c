#include <voxy/scene/main_game/update/chunk_generate.h>

#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/update/generate.h>

#include <voxy/core/thread_pool.h>
#include <voxy/core/log.h>

#include <voxy/utils.h>

static struct chunk_data *chunk_data_generate(ivec3_t position)
{
  struct chunk_data *chunk_data = malloc(sizeof *chunk_data);
  chunk_data->dirty = true;

  block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  generate_chunk_blocks(world_seed_get(), position, blocks);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        block_id_t block_id = blocks[z][y][x];
        const struct block_info *block_info = query_block_info(block_id);

        chunk_data->blocks[z][y][x].id = block_id;
        chunk_data->blocks[z][y][x].ether = block_info->ether;
        chunk_data->blocks[z][y][x].light_level = block_info->light_level;
      }

  DYNAMIC_ARRAY_INIT(chunk_data->entities);
  DYNAMIC_ARRAY_INIT(chunk_data->new_entities);

  return chunk_data;
}

struct chunk_generate_job
{
  struct thread_pool_job job;
  struct chunk *chunk;
};

static void chunk_generate_job_invoke(struct thread_pool_job *job)
{
  struct chunk_generate_job *chunk_generate_job = container_of(job, struct chunk_generate_job, job);
  struct chunk *chunk = chunk_generate_job->chunk;
  atomic_store_explicit(&chunk->new_data, chunk_data_generate(chunk->position), memory_order_release);
}

static void chunk_generate_job_destroy(struct thread_pool_job *job)
{
  free(job);
}

bool enqueue_generate_chunk(struct chunk *chunk)
{
  struct chunk_generate_job *job = malloc(sizeof *job);
  job->job.invoke = chunk_generate_job_invoke;
  job->job.destroy = chunk_generate_job_destroy;
  job->chunk = chunk;
  thread_pool_enqueue(&job->job);
  return true;
}
