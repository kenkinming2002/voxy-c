#include "world_generator.h"
#include "config.h"

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

////////////////////////
/// Thread Functions ///
////////////////////////
void *world_generator_create_section_infos(void *arg)
{
  struct world_generator *world_generator = arg;
  struct section_info    *section_info;
  bool                    shutdown;
  for(;;)
  {
    pthread_mutex_lock(&world_generator->section_infos_mutex);
    while(!(shutdown = atomic_load_explicit(&world_generator->thread_shutdown, memory_order_acquire)) && !world_generator->section_infos_pending)
      pthread_cond_wait(&world_generator->section_infos_cond, &world_generator->section_infos_mutex);

    if(shutdown)
      break;

    section_info                           = world_generator->section_infos_pending;
    world_generator->section_infos_pending = world_generator->section_infos_pending->next_list;
    pthread_mutex_unlock(&world_generator->section_infos_mutex);

    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        ivec2_t local_position  = ivec2(x, y);
        ivec2_t global_position = ivec2_add(ivec2_mul_scalar(section_info->position, CHUNK_WIDTH), local_position);
        section_info->heights[y][x] = get_height(world_generator->seed, global_position);
      }

    atomic_store_explicit(&section_info->done, true, memory_order_release);
  }

  pthread_mutex_unlock(&world_generator->section_infos_mutex);
  return NULL;
}

void *world_generator_create_chunk_infos(void *arg)
{
  struct world_generator *world_generator = arg;
  struct chunk_info      *chunk_info;
  bool                    shutdown;
  for(;;)
  {
    pthread_mutex_lock(&world_generator->chunk_infos_mutex);
    while(!(shutdown = atomic_load_explicit(&world_generator->thread_shutdown, memory_order_acquire)) && !world_generator->chunk_infos_pending)
      pthread_cond_wait(&world_generator->chunk_infos_cond, &world_generator->chunk_infos_mutex);

    if(shutdown)
      break;

    chunk_info                           = world_generator->chunk_infos_pending;
    world_generator->chunk_infos_pending = world_generator->chunk_infos_pending->next_list;
    pthread_mutex_unlock(&world_generator->chunk_infos_mutex);

    for(int z = 0; z<CHUNK_WIDTH; ++z)
      for(int y = 0; y<CHUNK_WIDTH; ++y)
        for(int x = 0; x<CHUNK_WIDTH; ++x)
        {
          ivec3_t local_position  = ivec3(x, y, z);
          ivec3_t global_position = ivec3_add(ivec3_mul_scalar(chunk_info->position, CHUNK_WIDTH), local_position);
          chunk_info->caves[z][y][x] = get_cave(world_generator->seed, global_position);
        }

    atomic_store_explicit(&chunk_info->done, true, memory_order_release);
  }

  pthread_mutex_unlock(&world_generator->chunk_infos_mutex);
  return NULL;
}

////////////
/// Init ///
////////////
void world_generator_init(struct world_generator *world_generator, seed_t seed)
{
  world_generator->seed            = seed;
  world_generator->player_spawned  = false;

  world_generator->thread_shutdown = false;
  world_generator->thread_count    = get_nprocs();

  section_info_hash_table_init(&world_generator->section_infos);
  chunk_info_hash_table_init(&world_generator->chunk_infos);

  world_generator->section_infos_pending = NULL;
  world_generator->chunk_infos_pending   = NULL;

  pthread_mutex_init(&world_generator->section_infos_mutex, NULL);
  pthread_mutex_init(&world_generator->chunk_infos_mutex,   NULL);

  pthread_cond_init(&world_generator->section_infos_cond, NULL);
  pthread_cond_init(&world_generator->chunk_infos_cond,   NULL);

  world_generator->section_infos_threads = malloc(world_generator->thread_count * sizeof *world_generator->section_infos_threads);
  world_generator->chunk_infos_threads   = malloc(world_generator->thread_count * sizeof *world_generator->chunk_infos_threads);

  for(int i=0; i<world_generator->thread_count; ++i)
  {
    pthread_create(&world_generator->section_infos_threads[i], NULL, &world_generator_create_section_infos, world_generator);
    pthread_create(&world_generator->chunk_infos_threads[i],   NULL, &world_generator_create_chunk_infos,   world_generator);
  }
}

void world_generator_deinit(struct world_generator *world_generator)
{
  atomic_store_explicit(&world_generator->thread_shutdown, true, memory_order_release);

  pthread_cond_broadcast(&world_generator->section_infos_cond);
  pthread_cond_broadcast(&world_generator->chunk_infos_cond);

  for(int i=0; i<world_generator->thread_count; ++i)
  {
    pthread_join(world_generator->section_infos_threads[i], NULL);
    pthread_join(world_generator->chunk_infos_threads[i],   NULL);
  }

  free(world_generator->section_infos_threads);
  free(world_generator->chunk_infos_threads);

  section_info_hash_table_dispose(&world_generator->section_infos);
  chunk_info_hash_table_dispose(&world_generator->chunk_infos);

  pthread_mutex_destroy(&world_generator->section_infos_mutex);
  pthread_mutex_destroy(&world_generator->chunk_infos_mutex);

  pthread_cond_destroy(&world_generator->section_infos_cond);
  pthread_cond_destroy(&world_generator->chunk_infos_cond);
}

void world_generator_update(struct world_generator *world_generator, struct world *world)
{
  world_generator_update_spawn_player(world_generator, world);
  world_generator_update_generate_chunks(world_generator, world);
}

void world_generator_update_spawn_player(struct world_generator *world_generator, struct world *world)
{
  if(!world_generator->player_spawned)
  {
    world_generator->player_spawned     = true;
    world->player_transform.translation = fvec3(0.0f, 0.0f, get_height(world->seed, ivec2(0, 0)) + 2.0f);
    world->player_transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);
    printf("Spawning player at (%f, %f, %f)\n",
        world->player_transform.translation.x,
        world->player_transform.translation.y,
        world->player_transform.translation.z);
  }
}

void world_generator_update_generate_chunks(struct world_generator *world_generator, struct world *world)
{
  struct section_info *section_info;
  struct chunk_info   *chunk_info;
  struct chunk        *chunk;

  ivec3_t player_chunk_position   = fvec3_as_ivec3_floor(fvec3_div_scalar(world->player_transform.translation, CHUNK_WIDTH));
  ivec2_t player_section_position = ivec2(player_chunk_position.x, player_chunk_position.y);
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        ivec2_t section_position = ivec2_add(player_section_position, ivec2(dx, dy));
        ivec3_t chunk_position   = ivec3_add(player_chunk_position,   ivec3(dx, dy, dz));

        ///////////////////////////////
        /// 1: Check if chunk exist ///
        ///////////////////////////////
        chunk = chunk_hash_table_lookup(&world->chunks, chunk_position);
        if(chunk)
          continue;

        /////////////////////////////////
        /// 2: Get Section/Chunk Info ///
        /////////////////////////////////
        section_info = section_info_hash_table_lookup(&world_generator->section_infos, section_position);
        if(!section_info)
        {
          struct section_info *section_info = malloc(sizeof *section_info);
          section_info->position = section_position;
          section_info->done     = false;
          section_info_hash_table_insert_unchecked(&world_generator->section_infos, section_info);

          pthread_mutex_lock(&world_generator->section_infos_mutex);
          section_info->next_list                = world_generator->section_infos_pending;
          world_generator->section_infos_pending = section_info;
          pthread_mutex_unlock(&world_generator->section_infos_mutex);
          pthread_cond_signal(&world_generator->section_infos_cond);
        }

        chunk_info = chunk_info_hash_table_lookup(&world_generator->chunk_infos, chunk_position);
        if(!chunk_info)
        {
          struct chunk_info *chunk_info = malloc(sizeof *chunk_info);
          chunk_info->position = chunk_position;
          chunk_info->done     = false;
          chunk_info_hash_table_insert_unchecked(&world_generator->chunk_infos, chunk_info);

          pthread_mutex_lock(&world_generator->chunk_infos_mutex);
          chunk_info->next_list                = world_generator->chunk_infos_pending;
          world_generator->chunk_infos_pending = chunk_info;
          pthread_mutex_unlock(&world_generator->chunk_infos_mutex);
          pthread_cond_signal(&world_generator->chunk_infos_cond);
        }

        if(!section_info || !atomic_load_explicit(&section_info->done, memory_order_acquire)) continue;
        if(!chunk_info   || !atomic_load_explicit(&chunk_info->done,   memory_order_acquire)) continue;

        //////////////////////////
        /// 3: Build the chunk ///
        //////////////////////////
        chunk = malloc(sizeof *chunk);
        chunk->position = chunk_position;

        for(int z = 0; z<CHUNK_WIDTH; ++z)
          for(int y = 0; y<CHUNK_WIDTH; ++y)
            for(int x = 0; x<CHUNK_WIDTH; ++x)
            {
              ivec3_t local_position  = ivec3(x, y, z);
              ivec3_t global_position = ivec3_add(ivec3_mul_scalar(chunk->position, CHUNK_WIDTH), local_position);

              chunk->tiles[z][y][x].id          = get_tile_id(chunk_info, section_info, global_position, local_position);
              chunk->tiles[z][y][x].ether       = false;
              chunk->tiles[z][y][x].light_level = 0;
            }

        world_chunk_insert_unchecked(world, chunk);

        ////////////////////////
        /// 4: Invalidations ///
        ////////////////////////
        chunk->mesh_dirty  = true;
        chunk->light_dirty = true;

        if(chunk->left)   chunk->left  ->mesh_dirty = true;
        if(chunk->right)  chunk->right ->mesh_dirty = true;
        if(chunk->back)   chunk->back  ->mesh_dirty = true;
        if(chunk->front)  chunk->front ->mesh_dirty = true;
        if(chunk->bottom) chunk->bottom->mesh_dirty = true;
        if(chunk->top)    chunk->top   ->mesh_dirty = true;
      }
}

