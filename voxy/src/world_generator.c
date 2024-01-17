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

static void vec2_rotate(struct vec2 *vec, float degree)
{
  float new_x = vec->x * +cosf(degree) + vec->y * +sinf(degree);
  float new_y = vec->x * -sinf(degree) + vec->y * +cosf(degree);
  vec->x = new_x;
  vec->y = new_y;
}

static float get_height(seed_t seed, struct ivec2 position)
{
  float value = 0.0f;

  seed_t      current_seed     = seed;
  struct vec2 current_position = vec2(position.x, position.y);

  // Truly Massive Mountains
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 8000.0f)) * 2048.0f + 2048.0f;
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 4000.0f)) * 1024.0f + 1024.0f;

  // Mountaincurrent_s
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 1600.0f)) * 512.0f + 512.0f;
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 800.0f))  * 256.0f + 256.0f;

  // Hills
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 400.0f)) * 256.0f + 256.0f;
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 200.0f)) * 128.0f + 128.0f;

  // Small hicurrent_lls
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 10.0f)) * 5.0f + 5.0f;
  seed_next(&current_seed); vec2_rotate(&current_position, M_PI / 4); value += noise_perlin2(current_seed, vec2_div_s(current_position, 5.0f))  * 2.0f + 2.0f;

  return value;
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
  }
}

void world_generator_update_generate_chunks(struct world_generator *world_generator, struct world *world)
{
  struct ivec3 chunk_position = vec3_as_ivec3_floor(vec3_div_s(world->player_transform.translation, CHUNK_WIDTH));
  for(int dz = -GENERATOR_DISTANCE; dz<=GENERATOR_DISTANCE; ++dz)
    for(int dy = -GENERATOR_DISTANCE; dy<=GENERATOR_DISTANCE; ++dy)
      for(int dx = -GENERATOR_DISTANCE; dx<=GENERATOR_DISTANCE; ++dx)
        world_generator_update_generate_chunk(world_generator, world, ivec3_add(chunk_position, ivec3(dx, dy, dz)));
}

void world_generator_update_generate_chunk(struct world_generator *world_generator, struct world *world, struct ivec3 chunk_position)
{
  if(!chunk_hash_table_lookup(&world->chunks, chunk_position))
  {
    struct chunk *chunk = malloc(sizeof *chunk);
    chunk->position = chunk_position;
    chunk->mesh_dirty = false;

    if(chunk->position.z < 0)
    {
      for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
        for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
          for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
            chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
    }
    else
    {
      // 1: Terrain Generation
      struct section_info *section_info = world_generator_section_info_get(world_generator, world, ivec2(chunk_position.x, chunk_position.y));
#pragma omp parallel for collapse(3)
      for(int z = 0; z<CHUNK_WIDTH; ++z)
        for(int y = 0; y<CHUNK_WIDTH; ++y)
          for(int x = 0; x<CHUNK_WIDTH; ++x)
          {
            struct ivec3 local_position  = ivec3(x, y, z);
            struct ivec3 global_position = ivec3_add(ivec3_mul_s(chunk->position, CHUNK_WIDTH), local_position);

            float threshold = (1.0f - expf(-chunk_position.z / 180.0f)) * 0.95f;
            seed_t seed = world->seed ^ 0xdeadbeefdeadbeef;

            bool cave = false;
            for(unsigned i=0; i<5; ++i)
            {
              bool inner_cave = true;
              for(unsigned j=0; j<3; ++j)
              {
                seed_next(&seed);

                float value = 0.0f;
                value += noise_perlin3(seed, vec3_div_s(ivec3_as_vec3(global_position), 30.0f)) * 2.0f + 2.0f;
                value += noise_perlin3(seed, vec3_div_s(ivec3_as_vec3(global_position), 20.0f)) * 1.0f + 1.0f;
                value += noise_perlin3(seed, vec3_div_s(ivec3_as_vec3(global_position), 10.0f)) * 0.5f + 0.5f;
                value /= 5.215f;
                if(value < threshold)
                {
                  inner_cave = false;
                  break;
                }
              }

              if(inner_cave)
              {
                cave = true;
                break;
              }
            }

            if(!cave)
            {
              if(global_position.z <= section_info->heights[y][x])
                chunk->tiles[z][y][x].id = TILE_ID_STONE;
              else if(global_position.z <= section_info->heights[y][x] + 1.0f)
                chunk->tiles[z][y][x].id = TILE_ID_GRASS;
              else
                chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
            }
            else
              chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
          }
    }
    chunk_hash_table_insert_unchecked(&world->chunks, chunk);

    struct chunk_adjacency chunk_adjacency;
    chunk_adjacency_init(&chunk_adjacency, world, chunk);
    if(chunk_adjacency.chunk)  chunk_adjacency.chunk ->mesh_dirty = true;
    if(chunk_adjacency.bottom) chunk_adjacency.bottom->mesh_dirty = true;
    if(chunk_adjacency.top)    chunk_adjacency.top   ->mesh_dirty = true;
    if(chunk_adjacency.back)   chunk_adjacency.back  ->mesh_dirty = true;
    if(chunk_adjacency.front)  chunk_adjacency.front ->mesh_dirty = true;
    if(chunk_adjacency.left)   chunk_adjacency.left  ->mesh_dirty = true;
    if(chunk_adjacency.right)  chunk_adjacency.right ->mesh_dirty = true;
  }
}

struct section_info *world_generator_section_info_get(struct world_generator *world_generator, struct world *world, struct ivec2 section_position)
{
  struct section_info *section_info = section_info_hash_table_lookup(&world_generator->section_infos, section_position);
  if(!section_info)
  {
    section_info = malloc(sizeof *section_info);
    section_info->position = section_position;
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        struct ivec2 local_position  = ivec2(x, y);
        struct ivec2 global_position = ivec2_add(ivec2_mul_s(section_info->position, CHUNK_WIDTH), local_position);
        section_info->heights[y][x] = get_height(world->seed, global_position);
      }

    section_info_hash_table_insert_unchecked(&world_generator->section_infos, section_info);
  }
  return section_info;
}

