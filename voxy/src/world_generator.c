#include "world_generator.h"
#include "hash.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX section_info
#define SC_HASH_TABLE_NODE_TYPE struct section_info
#define SC_HASH_TABLE_KEY_TYPE struct ivec2
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk_info
#define SC_HASH_TABLE_NODE_TYPE struct chunk_info
#define SC_HASH_TABLE_KEY_TYPE struct ivec3
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <stdlib.h>
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

struct ivec3 chunk_info_key(struct chunk_info *chunk_info)
{
  return chunk_info->position;
}

size_t chunk_info_hash(struct ivec3 position)
{
  return hash3(position.x, position.y, position.z);
}

int chunk_info_compare(struct ivec3 position1, struct ivec3 position2)
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

static void vec2_rotate(struct vec2 *vec, float degree)
{
  float new_x = vec->x * +cosf(degree) + vec->y * +sinf(degree);
  float new_y = vec->x * -sinf(degree) + vec->y * +cosf(degree);
  vec->x = new_x;
  vec->y = new_y;
}

static struct vec3 get_cave_direction(seed_t seed_x, seed_t seed_y, seed_t seed_z, struct vec3 position)
{
  struct vec3 direction = vec3_zero();

  direction.x += noise_perlin3(seed_x, vec3_div_s(position, 50.0f));
  direction.y += noise_perlin3(seed_y, vec3_div_s(position, 50.0f));
  direction.z += noise_perlin3(seed_z, vec3_div_s(position, 50.0f));

  direction.x += noise_perlin3(seed_x, vec3_div_s(position, 25.0f)) * 0.5f;
  direction.y += noise_perlin3(seed_y, vec3_div_s(position, 25.0f)) * 0.5f;
  direction.z += noise_perlin3(seed_z, vec3_div_s(position, 25.0f)) * 0.5f;

  return vec3_normalize(direction);
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
  chunk_info_hash_table_init(&world_generator->chunk_infos);

}

void world_generator_deinit(struct world_generator *world_generator)
{
  section_info_hash_table_dispose(&world_generator->section_infos);
  chunk_info_hash_table_dispose(&world_generator->chunk_infos);
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
  for(int dz = -8; dz<=8; ++dz)
    for(int dy = -8; dy<=8; ++dy)
      for(int dx = -8; dx<=8; ++dx)
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
      for(int z = 0; z<CHUNK_WIDTH; ++z)
        for(int y = 0; y<CHUNK_WIDTH; ++y)
          for(int x = 0; x<CHUNK_WIDTH; ++x)
          {
            int real_z = chunk->position.z * CHUNK_WIDTH + (int)z;
            if(real_z <= section_info->heights[y][x])
              chunk->tiles[z][y][x].id = TILE_ID_STONE;
            else if(real_z <= section_info->heights[y][x] + 1.0f)
              chunk->tiles[z][y][x].id = TILE_ID_GRASS;
            else
              chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
          }

      // 2: Cave generation
      struct chunk_info *chunk_info = world_generator_chunk_info_get(world_generator, world, chunk_position);
      for(size_t i=0; i<chunk_info->node_count; ++i)
        for(int z = floorf(chunk_info->nodes[i].position.z - CAVE_WORM_NODE_RADIUS); z <= ceilf(chunk_info->nodes[i].position.z + CAVE_WORM_NODE_RADIUS); ++z)
          for(int y = floorf(chunk_info->nodes[i].position.y - CAVE_WORM_NODE_RADIUS); y <= ceilf(chunk_info->nodes[i].position.y + CAVE_WORM_NODE_RADIUS); ++y)
            for(int x = floorf(chunk_info->nodes[i].position.x - CAVE_WORM_NODE_RADIUS); x <= ceilf(chunk_info->nodes[i].position.x + CAVE_WORM_NODE_RADIUS); ++x)
              if(vec3_length(vec3_sub(vec3(x, y, z), chunk_info->nodes[i].position)) <= CAVE_WORM_NODE_RADIUS)
                if(z >= 0 && z <= CHUNK_WIDTH - 1)
                  if(y >= 0 && y <= CHUNK_WIDTH - 1)
                    if(x >= 0 && x <= CHUNK_WIDTH - 1)
                      chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
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

struct chunk_info *world_generator_chunk_info_get(struct world_generator *world_generator, struct world *world, struct ivec3 chunk_position)
{
  int count = ceilf((CAVE_WORM_STEP * CAVE_WORM_NODE_COUNT + 2 * CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH);
  for(int dz = -count; dz<=count; ++dz)
    for(int dy = -count; dy<=count; ++dy)
      for(int dx = -count; dx<=count; ++dx)
        if(dx * dx + dy * dy + dz * dz <= count * count)
        {
          struct ivec3 delta_chunk_position     = ivec3(dx, dy, dz);
          struct ivec3 neighbour_chunk_position = ivec3_add(chunk_position, delta_chunk_position);
          world_generator_generate_cave(world_generator, world, neighbour_chunk_position);
        }

  return chunk_info_hash_table_lookup(&world_generator->chunk_infos, chunk_position);
}

void world_generator_generate_cave(struct world_generator *world_generator, struct world *world, struct ivec3 chunk_position)
{
  struct chunk_info *chunk_info = chunk_info_hash_table_lookup(&world_generator->chunk_infos, chunk_position);
  if(!chunk_info)
  {
    chunk_info = malloc(sizeof *chunk_info);
    chunk_info->position = chunk_position;
    chunk_info->cave_generated = false;
    chunk_info->nodes          = NULL;
    chunk_info->node_count     = 0;
    chunk_info->node_capacity  = 0;
    chunk_info_hash_table_insert_unchecked(&world_generator->chunk_infos, chunk_info);
  }

  if(!chunk_info->cave_generated)
  {
    seed_t seed = world->seed;
    seed_combine(&seed, &chunk_position.x, sizeof chunk_position.x);
    seed_combine(&seed, &chunk_position.y, sizeof chunk_position.y);
    seed_combine(&seed, &chunk_position.z, sizeof chunk_position.z);
    for(unsigned trial=0; trial<CAVE_WORM_TRIAL; ++trial)
    {
      float value = (float)seed_next(&seed) / (float)SEED_MAX;
      if(value < CAVE_WORM_RATIO)
      {
        seed_t seed_x = seed_next(&seed);
        seed_t seed_y = seed_next(&seed);
        seed_t seed_z = seed_next(&seed);

        struct ivec3 local_position  = seed_rand_ivec3(&seed, ivec3_zero(), ivec3(CHUNK_WIDTH-1, CHUNK_WIDTH-1, CHUNK_WIDTH-1));
        struct ivec3 global_position = ivec3_add(ivec3_mul_s(chunk_position, CHUNK_WIDTH), local_position);

        struct vec3 node_position = ivec3_as_vec3(global_position);
        world_generator_add_node(world_generator, node_position);
        for(size_t i=1; i<CAVE_WORM_NODE_COUNT; ++i)
        {
          node_position = vec3_add(node_position, vec3_mul_s(get_cave_direction(seed_x, seed_y, seed_z, node_position), CAVE_WORM_STEP));
          world_generator_add_node(world_generator, node_position);
        }
      }
    }
    chunk_info->cave_generated = true;
  }
}

void world_generator_add_node(struct world_generator *world_generator, struct vec3 position)
{
  for(int z = floorf((position.z - CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH); z<=ceilf((position.z + CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH); ++z)
    for(int y = floorf((position.y - CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH); y<=ceilf((position.y + CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH); ++y)
      for(int x = floorf((position.x - CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH); x<=ceilf((position.x + CAVE_WORM_NODE_RADIUS) / CHUNK_WIDTH); ++x)
      {
        struct ivec3 chunk_position = ivec3(x, y, z);
        struct chunk_info *chunk_info = chunk_info_hash_table_lookup(&world_generator->chunk_infos, chunk_position);
        if(!chunk_info)
        {
          chunk_info = malloc(sizeof *chunk_info);
          chunk_info->position = chunk_position;
          chunk_info->cave_generated = false;
          chunk_info->nodes          = NULL;
          chunk_info->node_count     = 0;
          chunk_info->node_capacity  = 0;
          chunk_info_hash_table_insert_unchecked(&world_generator->chunk_infos, chunk_info);
        }

        if(chunk_info->node_capacity == chunk_info->node_count)
        {
          chunk_info->node_capacity = chunk_info->node_capacity != 0 ? chunk_info->node_capacity * 2 : 1;
          chunk_info->nodes         = realloc(chunk_info->nodes, chunk_info->node_capacity * sizeof *chunk_info->nodes);
        }

        struct vec3 global_position = position;
        struct vec3 local_position  = vec3_sub(global_position, ivec3_as_vec3(ivec3_mul_s(chunk_position, CHUNK_WIDTH)));
        chunk_info->nodes[chunk_info->node_count++] = (struct node) { .position = local_position };
      }
}

