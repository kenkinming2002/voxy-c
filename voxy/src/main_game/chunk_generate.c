#include <voxy/main_game/chunk_generate.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/world_seed.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>

#include <voxy/types/chunk.h>
#include <voxy/types/chunk_data.h>
#include <voxy/types/player.h>

#include <voxy/core/thread_pool.h>

#include <voxy/config.h>
#include <voxy/utils.h>

#include <sc/hash_table.h>

#include <stdio.h>

struct chunk_generate_wrapper
{
  struct chunk_generate_wrapper *next;
  size_t                         hash;
  ivec3_t                        position;

  struct thread_pool_job job;

  atomic_bool done;
  uint8_t     blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
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
  mod_generate_blocks(world_seed_get(), wrapper->position, wrapper->blocks);
  atomic_store_explicit(&wrapper->done, true, memory_order_release);
}

void chunk_generate_wrapper_destroy(struct thread_pool_job *job)
{
  (void)job;
}

static struct chunk_generate_wrapper_hash_table chunk_generate_wrappers;

void update_chunk_generate(void)
{
  size_t count = 0;

  struct player *player = player_get();
  if(!player)
    return;

  struct entity *player_entity   = player_as_entity(player);
  ivec3_t        player_position = fvec3_as_ivec3_floor(fvec3_div_scalar(player_entity->position, CHUNK_WIDTH));

  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        ivec3_t position = ivec3_add(player_position, ivec3(dx, dy, dz));
        if(world_chunk_lookup(position))
          continue;

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
          continue;
        }

        if(!atomic_load_explicit(&wrapper->done, memory_order_acquire))
          continue;

        struct chunk *chunk = world_chunk_create(wrapper->position);
        for(int z = 0; z<CHUNK_WIDTH; ++z)
          for(int y = 0; y<CHUNK_WIDTH; ++y)
            for(int x = 0; x<CHUNK_WIDTH; ++x)
            {
              chunk->blocks[z][y][x].id          = wrapper->blocks[z][y][x];
              chunk->blocks[z][y][x].ether       = false;
              chunk->blocks[z][y][x].light_level = 0;
            }

        world_chunk_invalidate_mesh(chunk);
        world_chunk_invalidate_mesh(chunk->left);
        world_chunk_invalidate_mesh(chunk->right);
        world_chunk_invalidate_mesh(chunk->back);
        world_chunk_invalidate_mesh(chunk->front);
        world_chunk_invalidate_mesh(chunk->bottom);
        world_chunk_invalidate_mesh(chunk->top);
        world_chunk_invalidate_light(chunk);

        free(chunk_generate_wrapper_hash_table_remove(&chunk_generate_wrappers, position));

        count += 1;
      }

  if(count != 0)
    fprintf(stderr, "DEBUG: Chunk Generator: Generarted %zu chunks in background\n", count);
}
