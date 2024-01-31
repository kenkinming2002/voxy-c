#include "world_generator.h"

#include "noise.h"
#include "world.h"

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
#define SC_HASH_TABLE_PREFIX chunk_info
#define SC_HASH_TABLE_NODE_TYPE struct chunk_info
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
ivec3_t chunk_info_key(struct chunk_info *chunk_info)
{
  return chunk_info->position;
}

size_t chunk_info_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int chunk_info_compare(ivec3_t position1, ivec3_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_info_dispose(struct chunk_info *chunk_info)
{
  free(chunk_info);
}

void world_generator_init(struct world_generator *world_generator, seed_t seed)
{
  world_generator->seed = seed;
  thread_pool_init(&world_generator->thread_pool);
  section_info_hash_table_init(&world_generator->section_infos);
  chunk_info_hash_table_init(&world_generator->chunk_infos);
}

void world_generator_fini(struct world_generator *world_generator)
{
  thread_pool_fini(&world_generator->thread_pool);
  section_info_hash_table_dispose(&world_generator->section_infos);
  chunk_info_hash_table_dispose(&world_generator->chunk_infos);
}

/////////////////
/// Functions ///
/////////////////
static float lerpf(float a, float b, float t)
{
  return a + (b - a) * t;
}

static float get_height(seed_t seed, ivec2_t position)
{
  float value1 = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 1/200.0f, 2.3f, 0.4f, 8);
  value1 = fabs(value1);
  value1 = powf(value1, 2.0f);
  value1 = ease_out(value1, 3.0f);
  value1 = value1 * 100.0f;

  float value2 = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 1/2000.0f, 2.1f, 0.6f, 8);
  value2 = 0.5f * (value2 + 1.0f);
  value2 = smooth_step(value2);

  return value1 * value2;
}

static bool get_cave(seed_t seed, ivec3_t position)
{
  // Reference: https://blog.danol.cz/voxel-cave-generation-using-3d-perlin-noise-isosurfaces/
  float threshold = lerpf(0.0f, 0.1f, 1.0f/(1.0f+expf(position.z/1000.0f)));
  for(unsigned i=0; i<2; ++i)
  {
    float value = noise_perlin3_ex(seed_next(&seed), ivec3_as_fvec3(position), 0.02f, 1.5f, 0.3f, 4);
    if(fabs(value) > threshold)
      return false;
  }
  return true;
}

static uint8_t get_tile_id(struct chunk_info *chunk_info, struct section_info *section_info, ivec3_t global_position, ivec3_t local_position)
{
  if(global_position.z >= ETHER_HEIGHT)
    return TILE_ID_ETHER;

  if(global_position.z > section_info->heights[local_position.y][local_position.x] && global_position.z <= 3.0f)
    return TILE_ID_WATER;

  if(chunk_info->caves[local_position.z][local_position.y][local_position.x])
    return TILE_ID_EMPTY;

  if(global_position.z <= section_info->heights[local_position.y][local_position.x])
    return TILE_ID_STONE;

  if(global_position.z <= section_info->heights[local_position.y][local_position.x] + 1.0f)
    return TILE_ID_GRASS;

  return TILE_ID_EMPTY;
}

struct section_info_generate_info
{
  seed_t               seed;
  struct section_info *section_info;
};

struct chunk_info_generate_info
{
  seed_t             seed;
  struct chunk_info *chunk_info;
};

static void world_generator_generate_section_info_job(void *arg)
{
  seed_t               seed         = ((struct section_info_generate_info *)arg)->seed;
  struct section_info *section_info = ((struct section_info_generate_info *)arg)->section_info;

  for(int y = 0; y<CHUNK_WIDTH; ++y)
    for(int x = 0; x<CHUNK_WIDTH; ++x)
    {
      ivec2_t local_position  = ivec2(x, y);
      ivec2_t global_position = ivec2_add(ivec2_mul_scalar(section_info->position, CHUNK_WIDTH), local_position);
      section_info->heights[y][x] = get_height(seed, global_position);
    }

  atomic_store_explicit(&section_info->done, true, memory_order_release);
}

static void world_generator_generate_chunk_info_job(void *arg)
{
  seed_t             seed       = ((struct chunk_info_generate_info *)arg)->seed;
  struct chunk_info *chunk_info = ((struct chunk_info_generate_info *)arg)->chunk_info;

  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        ivec3_t local_position  = ivec3(x, y, z);
        ivec3_t global_position = ivec3_add(ivec3_mul_scalar(chunk_info->position, CHUNK_WIDTH), local_position);
        chunk_info->caves[z][y][x] = get_cave(seed, global_position);
      }

  atomic_store_explicit(&chunk_info->done, true, memory_order_release);
}

struct section_info *world_generator_generate_section_info(struct world_generator *world_generator, ivec2_t position)
{
  struct section_info *section_info;;
  if((section_info = section_info_hash_table_lookup(&world_generator->section_infos, position)))
    return atomic_load_explicit(&section_info->done, memory_order_acquire) ? section_info : NULL;

  section_info = malloc(sizeof *section_info);
  section_info->position = position;
  section_info->done     = false;
  section_info_hash_table_insert_unchecked(&world_generator->section_infos, section_info);

  struct section_info_generate_info *section_info_generate_info = malloc(sizeof *section_info_generate_info);
  section_info_generate_info->seed         = world_generator->seed;
  section_info_generate_info->section_info = section_info;
  thread_pool_enqueue(&world_generator->thread_pool, world_generator_generate_section_info_job, section_info_generate_info);

  return NULL;
}

struct chunk_info *world_generator_generate_chunk_info(struct world_generator *world_generator, ivec3_t position)
{
  struct chunk_info *chunk_info;;
  if((chunk_info = chunk_info_hash_table_lookup(&world_generator->chunk_infos, position)))
    return atomic_load_explicit(&chunk_info->done, memory_order_acquire) ? chunk_info : NULL;

  chunk_info = malloc(sizeof *chunk_info);
  chunk_info->position = position;
  chunk_info->done     = false;
  chunk_info_hash_table_insert_unchecked(&world_generator->chunk_infos, chunk_info);

  struct chunk_info_generate_info *chunk_info_generate_info = malloc(sizeof *chunk_info_generate_info);
  chunk_info_generate_info->seed         = world_generator->seed;
  chunk_info_generate_info->chunk_info = chunk_info;
  thread_pool_enqueue(&world_generator->thread_pool, world_generator_generate_chunk_info_job, chunk_info_generate_info);

  return NULL;
}

struct chunk_data *world_generator_generate_chunk_data(struct world_generator *world_generator, ivec3_t position)
{
  struct section_info *section_info = world_generator_generate_section_info(world_generator, ivec2(position.x, position.y));
  struct chunk_info   *chunk_info   = world_generator_generate_chunk_info(world_generator, position);
  if(section_info && chunk_info)
  {
    // FIXME: Put me in a thread pool
    struct chunk_data *chunk_data = malloc(sizeof *chunk_data);
    for(int z = 0; z<CHUNK_WIDTH; ++z)
      for(int y = 0; y<CHUNK_WIDTH; ++y)
        for(int x = 0; x<CHUNK_WIDTH; ++x)
        {
          ivec3_t local_position  = ivec3(x, y, z);
          ivec3_t global_position = ivec3_add(ivec3_mul_scalar(position, CHUNK_WIDTH), local_position);

          chunk_data->tiles[z][y][x].id          = get_tile_id(chunk_info, section_info, global_position, local_position);
          chunk_data->tiles[z][y][x].ether       = false;
          chunk_data->tiles[z][y][x].light_level = 0;
        }

    return chunk_data;
  }
  return NULL;
}

float world_generator_get_height(struct world_generator *world_generator, ivec2_t position)
{
  return get_height(world_generator->seed, position);
}
