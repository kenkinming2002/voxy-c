#include "world_generator.h"
#include "hash.h"
#include "config.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX section_info
#define SC_HASH_TABLE_NODE_TYPE struct section_info
#define SC_HASH_TABLE_KEY_TYPE struct ivec2
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <math.h>
#include <stdio.h>

struct ivec2 section_info_key(struct section_info *section_info)
{
  return section_info->position;
}

size_t section_info_hash(struct ivec2 position)
{
  return hash2(position.x, position.y);
}

int section_info_compare(struct ivec2 position1, struct ivec2 position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  return 0;
}

void section_info_dispose(struct section_info *section_info)
{
  free(section_info);
}

static float lerpf(float a, float b, float t)
{
  return a + (b - a) * t;
}

static float get_height(seed_t seed, struct ivec2 position)
{
  float value1 = noise_perlin2_ex(seed_next(&seed), ivec2_as_vec2(position), 1/200.0f, 2.3f, 0.4f, 8);
  value1 = fabs(value1);
  value1 = powf(value1, 1.5f);
  value1 = ease_out(value1, 3.0f);
  value1 = value1 * 100.0f;

  float value2 = noise_perlin2_ex(seed_next(&seed), ivec2_as_vec2(position), 1/2000.0f, 2.1f, 0.6f, 8);
  value2 = 0.5f * (value2 + 1.0f);
  value2 = smooth_step(value2);

  return value1 * value2;
}

static bool get_cave(seed_t seed, struct ivec3 position)
{
  // Reference: https://blog.danol.cz/voxel-cave-generation-using-3d-perlin-noise-isosurfaces/
  float threshold = lerpf(0.0f, 0.025f, 1.0f/(1.0f+expf(position.z/1000.0f)));
  for(unsigned i=0; i<2; ++i)
  {
    float value = noise_perlin3_ex(seed_next(&seed), ivec3_as_vec3(position), 0.013f, 1.5f, 0.3f, 4);
    if(fabs(value) > threshold)
      return false;
  }
  return true;
}

void world_generator_init(struct world_generator *world_generator)
{
  world_generator->player_spawned = false;
  section_info_hash_table_init(&world_generator->section_infos);
}

void world_generator_deinit(struct world_generator *world_generator)
{
  section_info_hash_table_dispose(&world_generator->section_infos);
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
    world->player_transform.translation = vec3(0.0f, 0.0f, get_height(world->seed, ivec2(0, 0)) + 2.0f);
    world->player_transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
    printf("Spawning player at (%f, %f, %f)\n",
        world->player_transform.translation.x,
        world->player_transform.translation.y,
        world->player_transform.translation.z);
  }
}

void world_generator_update_generate_chunks(struct world_generator *world_generator, struct world *world)
{
  struct section_info **section_infos         = NULL;
  size_t                section_info_count    = 0;
  size_t                section_info_capacity = 0;

  struct chunk **chunks         = NULL;
  size_t         chunk_count    = 0;
  size_t         chunk_capacity = 0;

  struct ivec3 player_chunk_position   = vec3_as_ivec3_floor(vec3_div_s(world->player_transform.translation, CHUNK_WIDTH));
  struct ivec2 player_section_position = ivec2(player_chunk_position.x, player_chunk_position.y);

  seed_t cave_seed = world->seed ^ 0xdeadbeefdeadbeef;

  /////////////////////////////////////////////////////////////
  // 1: Collect all section infos that need to be generated ///
  /////////////////////////////////////////////////////////////
  for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
    for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
    {
      struct ivec2 section_position = ivec2_add(player_section_position, ivec2(dx, dy));
      if(!section_info_hash_table_lookup(&world_generator->section_infos, section_position))
      {
        struct section_info *section_info = malloc(sizeof *section_info);
        section_info->position = section_position;
        section_info_hash_table_insert_unchecked(&world_generator->section_infos, section_info);

        if(section_info_capacity == section_info_count)
        {
          section_info_capacity = section_info_capacity != 0 ? section_info_capacity * 2 : 1;
          section_infos         = realloc(section_infos, section_info_capacity * sizeof *section_infos);
        }
        section_infos[section_info_count++] = section_info;
      }
    }

  //////////////////////////////////////////////////////
  // 2: Collect all chunks that need to be generated ///
  //////////////////////////////////////////////////////
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
      {
        struct ivec3 chunk_position = ivec3_add(player_chunk_position, ivec3(dx, dy, dz));
        if(!chunk_hash_table_lookup(&world->chunks, chunk_position))
        {
          struct chunk *chunk = malloc(sizeof *chunk);
          chunk->position   = chunk_position;
          chunk->mesh_dirty = true;
          chunk_hash_table_insert_unchecked(&world->chunks, chunk);

          if(chunk_capacity == chunk_count)
          {
            chunk_capacity = chunk_capacity != 0 ? chunk_capacity * 2 : 1;
            chunks         = realloc(chunks, chunk_capacity * sizeof *chunks);
          }
          chunks[chunk_count++] = chunk;
        }
      }

  ////////////////////////////////////////////////
  // 3: Generate all section infos in parallel ///
  ////////////////////////////////////////////////
  #pragma omp parallel for
  for(size_t i=0; i<section_info_count; ++i)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        struct ivec2 local_position  = ivec2(x, y);
        struct ivec2 global_position = ivec2_add(ivec2_mul_s(section_infos[i]->position, CHUNK_WIDTH), local_position);
        section_infos[i]->heights[y][x] = get_height(world->seed, global_position);
      }


  /////////////////////////////////////////
  // 4: Generate all chunks in parallel ///
  /////////////////////////////////////////
  #pragma omp parallel for
  for(size_t i=0; i<chunk_count; ++i)
  {
    struct section_info *section_info = section_info_hash_table_lookup(&world_generator->section_infos, ivec2(chunks[i]->position.x, chunks[i]->position.y));
    for(int z = 0; z<CHUNK_WIDTH; ++z)
      for(int y = 0; y<CHUNK_WIDTH; ++y)
        for(int x = 0; x<CHUNK_WIDTH; ++x)
        {
          struct ivec3 local_position  = ivec3(x, y, z);
          struct ivec3 global_position = ivec3_add(ivec3_mul_s(chunks[i]->position, CHUNK_WIDTH), local_position);

          if(global_position.z > section_info->heights[y][x] && global_position.z <= 3.0f) { chunks[i]->tiles[z][y][x].id = TILE_ID_WATER; continue; }

          if(get_cave(cave_seed, global_position)) { chunks[i]->tiles[z][y][x].id = TILE_ID_EMPTY; continue; }

          if(global_position.z <= section_info->heights[y][x])        { chunks[i]->tiles[z][y][x].id = TILE_ID_STONE; continue; }
          if(global_position.z <= section_info->heights[y][x] + 1.0f) { chunks[i]->tiles[z][y][x].id = TILE_ID_GRASS; continue; }

          chunks[i]->tiles[z][y][x].id = TILE_ID_EMPTY;
        }
  }

  ///////////////////////////
  // 5: Invalidate meshes ///
  ///////////////////////////
  for(size_t i=0; i<chunk_count; ++i)
  {
    struct chunk_adjacency chunk_adjacency;
    chunk_adjacency_init(&chunk_adjacency, world, chunks[i]);
    if(chunk_adjacency.bottom) chunk_adjacency.bottom->mesh_dirty = true;
    if(chunk_adjacency.top)    chunk_adjacency.top   ->mesh_dirty = true;
    if(chunk_adjacency.back)   chunk_adjacency.back  ->mesh_dirty = true;
    if(chunk_adjacency.front)  chunk_adjacency.front ->mesh_dirty = true;
    if(chunk_adjacency.left)   chunk_adjacency.left  ->mesh_dirty = true;
    if(chunk_adjacency.right)  chunk_adjacency.right ->mesh_dirty = true;
  }

  /////////////////
  // 5: Cleanup ///
  /////////////////
  free(section_infos);
  free(chunks);
}

