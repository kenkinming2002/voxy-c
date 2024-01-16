#include "world_generator.h"

#include <stdlib.h>
#include <stdio.h>

void world_generator_init(struct world_generator *world_generator)
{
  world_generator->player_spawned = false;

  world_generator->section_infos         = NULL;
  world_generator->section_info_capacity = 0;
  world_generator->section_info_load     = 0;
}

void world_generator_deinit(struct world_generator *world_generator)
{
  for(size_t i=0; i<world_generator->section_info_capacity; ++i)
  {
    struct section_info *section_info = world_generator->section_infos[i];
    while(section_info)
    {
      struct section_info *tmp = section_info;
      section_info = section_info->next;
      free(tmp);
    }
  }
  free(world_generator->section_infos);

  for(size_t i=0; i<world_generator->chunk_info_capacity; ++i)
  {
    struct chunk_info *chunk_info = world_generator->chunk_infos[i];
    while(chunk_info)
    {
      struct chunk_info *tmp = chunk_info;
      chunk_info = chunk_info->next;

      free(tmp->worms);
      free(tmp);
    }
  }
  free(world_generator->chunk_infos);
}

static inline size_t hash2(int x, int y)
{
  return x * 23 + y * 31;
}

static inline size_t hash3(int x, int y, int z)
{
  return x * 23 + y * 31 + z * 41;
}

void world_generator_section_info_rehash(struct world_generator *world_generator, size_t new_capacity)
{
  struct section_info *orphans = NULL;
  for(size_t i=0; i<world_generator->section_info_capacity; ++i)
  {
    struct section_info **head = &world_generator->section_infos[i];
    while(*head)
      if((*head)->hash % new_capacity != i)
      {
        struct section_info *orphan = *head;
        *head = (*head)->next;

        orphan->next = orphans;
        orphans      = orphan;
      }
      else
        head = &(*head)->next;
  }

  world_generator->section_infos = realloc(world_generator->section_infos, new_capacity * sizeof *world_generator->section_infos);
  for(size_t i=world_generator->section_info_capacity; i<new_capacity; ++i)
    world_generator->section_infos[i] = NULL;
  world_generator->section_info_capacity = new_capacity;

  while(orphans)
  {
    struct section_info *orphan = orphans;
    orphans = orphans->next;

    orphan->next = world_generator->section_infos[orphan->hash % world_generator->section_info_capacity];
    world_generator->section_infos[orphan->hash % world_generator->section_info_capacity] = orphan;
  }
}

struct section_info *world_generator_section_info_insert(struct world_generator *world_generator, int x, int y)
{
  if(world_generator->section_info_capacity == 0)
    world_generator_section_info_rehash(world_generator, 32);
  else if(world_generator->section_info_load * 4 >= world_generator->section_info_capacity * 3)
    world_generator_section_info_rehash(world_generator, world_generator->section_info_capacity * 2);

  struct section_info *section_info = malloc(sizeof *section_info);
  section_info->x = x;
  section_info->y = y;
  section_info->hash = hash2(x, y);
  section_info->next = world_generator->section_infos[section_info->hash % world_generator->section_info_capacity];
  world_generator->section_infos[section_info->hash % world_generator->section_info_capacity] = section_info;
  world_generator->section_info_load += 1;
  return section_info;
}

struct section_info *world_generator_section_info_lookup(struct world_generator *world_generator, int x, int y)
{
  if(world_generator->section_info_capacity == 0)
    return NULL;

  size_t h = hash2(x, y);
  for(struct section_info *section_info = world_generator->section_infos[h % world_generator->section_info_capacity]; section_info; section_info = section_info->next)
    if(section_info->hash == h && section_info->x == x && section_info->y == y)
      return section_info;
  return NULL;
}

static void vec2_rotate(struct vec2 *vec, float degree)
{
  float new_x = vec->x * +cosf(degree) + vec->y * +sinf(degree);
  float new_y = vec->x * -sinf(degree) + vec->y * +cosf(degree);
  vec->x = new_x;
  vec->y = new_y;
}

static float get_height(seed_t seed, int x, int y)
{
  float value = 0.0f;

  struct vec2 position = vec2(x, y);

  // Truly Massive Mountains
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 8000.0f)) * 2048.0f + 2048.0f;
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 4000.0f)) * 1024.0f + 1024.0f;

  // Mountains
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 1600.0f)) * 512.0f + 512.0f;
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 800.0f))  * 256.0f + 256.0f;

  // Hills
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 400.0f)) * 256.0f + 256.0f;
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 200.0f)) * 128.0f + 128.0f;

  // Small hills
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 10.0f)) * 5.0f + 5.0f;
  seed_next(&seed); vec2_rotate(&position, M_PI / 4); value += perlin2(seed, vec2_div_s(position, 5.0f))  * 2.0f + 2.0f;

  return value;
}

struct section_info *world_generator_section_info_get(struct world_generator *world_generator, struct world *world, int x, int y)
{
  struct section_info *section_info;
  if((section_info = world_generator_section_info_lookup(world_generator, x, y)))
    return section_info;

  section_info = world_generator_section_info_insert(world_generator, x, y);
  for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
    for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
    {
      int real_x = section_info->x * CHUNK_WIDTH + (int)x;
      int real_y = section_info->y * CHUNK_WIDTH + (int)y;
      section_info->heights[y][x] = get_height(world->seed, real_x, real_y);
    }

  return section_info;
}

void world_generator_chunk_info_rehash(struct world_generator *world_generator, size_t new_capacity)
{
  struct chunk_info *orphans = NULL;
  for(size_t i=0; i<world_generator->chunk_info_capacity; ++i)
  {
    struct chunk_info **head = &world_generator->chunk_infos[i];
    while(*head)
      if((*head)->hash % new_capacity != i)
      {
        struct chunk_info *orphan = *head;
        *head = (*head)->next;

        orphan->next = orphans;
        orphans      = orphan;
      }
      else
        head = &(*head)->next;
  }

  world_generator->chunk_infos = realloc(world_generator->chunk_infos, new_capacity * sizeof *world_generator->chunk_infos);
  for(size_t i=world_generator->chunk_info_capacity; i<new_capacity; ++i)
    world_generator->chunk_infos[i] = NULL;
  world_generator->chunk_info_capacity = new_capacity;

  while(orphans)
  {
    struct chunk_info *orphan = orphans;
    orphans = orphans->next;

    orphan->next = world_generator->chunk_infos[orphan->hash % world_generator->chunk_info_capacity];
    world_generator->chunk_infos[orphan->hash % world_generator->chunk_info_capacity] = orphan;
  }
}

struct chunk_info *world_generator_chunk_info_insert(struct world_generator *world_generator, int x, int y, int z)
{
  if(world_generator->chunk_info_capacity == 0)
    world_generator_chunk_info_rehash(world_generator, 32);
  else if(world_generator->chunk_info_load * 4 >= world_generator->chunk_info_capacity * 3)
    world_generator_chunk_info_rehash(world_generator, world_generator->chunk_info_capacity * 2);

  struct chunk_info *chunk_info = malloc(sizeof *chunk_info);
  chunk_info->x = x;
  chunk_info->y = y;
  chunk_info->z = z;
  chunk_info->hash = hash3(x, y, z);
  chunk_info->next = world_generator->chunk_infos[chunk_info->hash % world_generator->chunk_info_capacity];
  world_generator->chunk_infos[chunk_info->hash % world_generator->chunk_info_capacity] = chunk_info;
  world_generator->chunk_info_load += 1;
  return chunk_info;
}

struct chunk_info *world_generator_chunk_info_lookup(struct world_generator *world_generator, int x, int y, int z)
{
  if(world_generator->chunk_info_capacity == 0)
    return NULL;

  size_t h = hash3(x, y, z);
  for(struct chunk_info *chunk_info = world_generator->chunk_infos[h % world_generator->chunk_info_capacity]; chunk_info; chunk_info = chunk_info->next)
    if(chunk_info->hash == h && chunk_info->x == x && chunk_info->y == y)
      return chunk_info;
  return NULL;
}

struct chunk_info *world_generator_chunk_info_get(struct world_generator *world_generator, struct world *world, int x, int y, int z)
{
  struct chunk_info *chunk_info;
  if((chunk_info = world_generator_chunk_info_lookup(world_generator, x, y, z)))
    return chunk_info;

  chunk_info = world_generator_chunk_info_insert(world_generator, x, y, z);

  seed_t seed = world->seed;

  seed_t seed_gen = seed_next(&seed); // Determine whether to generate a worm
  seed_t seed_x   = seed_next(&seed); // Determine x direction for worm movement
  seed_t seed_y   = seed_next(&seed); // Determine y direction for worm movement
  seed_t seed_z   = seed_next(&seed); // Determine z direction for worm movement

  struct worm *worms         = NULL;
  size_t       worm_count    = 0;
  size_t       worm_capacity = 0;
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        int real_x = chunk_info->x * CHUNK_WIDTH + x;
        int real_y = chunk_info->y * CHUNK_WIDTH + y;
        int real_z = chunk_info->z * CHUNK_WIDTH + z;
        if(random3(seed_gen, vec3(real_x, real_y, real_z)) < CAVE_WORM_RATIO)
        {
          if(worm_capacity == worm_count)
          {
            worm_capacity = worm_capacity != 0 ? worm_capacity * 2 : 1;
            worms         = realloc(worms, worm_capacity * sizeof *worms);
          }

          struct worm *worm = &worms[worm_count++];
          worm->nodes[0] = vec3(real_x, real_y, real_z);
          for(size_t i=1; i<CAVE_WORM_NODE_COUNT; ++i)
          {
            struct vec3 offset = vec3(perlin3(seed_x, vec3_div_s(worm->nodes[i-1], 15.0f)),
                                      perlin3(seed_y, vec3_div_s(worm->nodes[i-1], 15.0f)),
                                      perlin3(seed_z, vec3_div_s(worm->nodes[i-1], 15.0f)));

            offset = vec3_normalize(offset);
            offset = vec3_mul_s(offset, CAVE_WORM_STEP);

            worm->nodes[i] = vec3_add(worms->nodes[i-1], offset);
          }
        }
      }

  chunk_info->worms      = realloc(worms, worm_count * sizeof *worms);
  chunk_info->worm_count = worm_count;
  return chunk_info;
}

void world_generator_update(struct world_generator *world_generator, struct world *world)
{
  if(!world_generator->player_spawned)
  {
    world_generator->player_spawned     = true;
    world->player_transform.translation = vec3(0.5f, 0.5f, get_height(world->seed, 0, 0) + 2.0f);
    world->player_transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
  }

  int x = floorf(world->player_transform.translation.x / CHUNK_WIDTH);
  int y = floorf(world->player_transform.translation.y / CHUNK_WIDTH);
  int z = floorf(world->player_transform.translation.z / CHUNK_WIDTH);
  for(int dz = -8; dz<=8; ++dz)
    for(int dy = -8; dy<=8; ++dy)
      for(int dx = -8; dx<=8; ++dx)
        if(!world_chunk_lookup(world, x+dx, y+dy, z+dz))
          world_generator_update_at(world_generator, world, x+dx, y+dy, z+dz);
}

void world_generator_update_at(struct world_generator *world_generator, struct world *world, int x, int y, int z)
{
  (void)world_generator;

  struct chunk *chunk = world_chunk_insert(world, x, y, z);
  chunk->mesh_dirty = false;

  if(chunk->z < 0)
  {
    for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
      for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
        for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;

    return; // Fast-path!?
  }

  // 1: Terrain Generation
  struct section_info *section_info = world_generator_section_info_get(world_generator, world, x, y);
  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        int real_z = chunk->z * CHUNK_WIDTH + (int)z;
        if(real_z <= section_info->heights[y][x])
          chunk->tiles[z][y][x].id = TILE_ID_STONE;
        else if(real_z <= section_info->heights[y][x] + 1.0f)
          chunk->tiles[z][y][x].id = TILE_ID_GRASS;
        else
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
      }

  // 2: Cave generation
  int count = CAVE_WORM_STEP * CAVE_WORM_NODE_COUNT / CHUNK_WIDTH;
  for(int dz = -count; dz<=count; ++dz)
    for(int dy = -count; dy<=count; ++dy)
      for(int dx = -count; dx<=count; ++dx)
      {
        struct chunk_info *chunk_info = world_generator_chunk_info_get(world_generator, world, x+dx, y+dy, z+dz);
        for(size_t i=0; i<chunk_info->worm_count; ++i)
          for(size_t j=0; j<CAVE_WORM_NODE_COUNT; ++j)
          {
            struct vec3 node = vec3_sub(chunk_info->worms[i].nodes[j], vec3_mul_s(vec3(x, y, z), CHUNK_WIDTH));
            for(int cz = floorf(node.z - CAVE_WORM_NODE_RADIUS); cz <= ceilf(node.z + CAVE_WORM_NODE_RADIUS); ++cz)
              for(int cy = floorf(node.y - CAVE_WORM_NODE_RADIUS); cy <= ceilf(node.y + CAVE_WORM_NODE_RADIUS); ++cy)
                for(int cx = floorf(node.x - CAVE_WORM_NODE_RADIUS); cx <= ceilf(node.x + CAVE_WORM_NODE_RADIUS); ++cx)
                  if(vec3_length(vec3_sub(vec3(cx, cy, cz), node)) <= CAVE_WORM_NODE_RADIUS)
                    if(cz >= 0 && cz < CHUNK_WIDTH)
                      if(cy >= 0 && cy < CHUNK_WIDTH)
                        if(cx >= 0 && cx < CHUNK_WIDTH)
                          chunk->tiles[cz][cy][cx].id = TILE_ID_EMPTY;
          }
      }



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

