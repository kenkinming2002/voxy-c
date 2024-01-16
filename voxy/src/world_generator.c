#include "world_generator.h"

#include <stdlib.h>

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
}

static inline size_t hash(int x, int y)
{
  // I honestly do not know what I am doing here
  return x * 23 + y * 31;
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

struct section_info *world_generator_section_info_add(struct world_generator *world_generator, int x, int y)
{
  if(world_generator->section_info_capacity == 0)
    world_generator_section_info_rehash(world_generator, 32);
  else if(world_generator->section_info_load * 4 >= world_generator->section_info_capacity * 3)
    world_generator_section_info_rehash(world_generator, world_generator->section_info_capacity * 2);

  struct section_info *section_info = malloc(sizeof *section_info);
  section_info->x = x;
  section_info->y = y;
  section_info->hash = hash(x, y);
  section_info->next = world_generator->section_infos[section_info->hash % world_generator->section_info_capacity];
  world_generator->section_infos[section_info->hash % world_generator->section_info_capacity] = section_info;
  world_generator->section_info_load += 1;
  return section_info;
}

struct section_info *world_generator_section_info_lookup(struct world_generator *world_generator, int x, int y)
{
  if(world_generator->section_info_capacity == 0)
    return NULL;

  size_t h = hash(x, y);
  for(struct section_info *section_info = world_generator->section_infos[h % world_generator->section_info_capacity]; section_info; section_info = section_info->next)
    if(section_info->hash == h && section_info->x == x && section_info->y == y)
      return section_info;
  return NULL;
}

struct section_info *world_generator_section_info_get(struct world_generator *world_generator, struct world *world, int x, int y)
{
  struct section_info *section_info;
  if((section_info = world_generator_section_info_lookup(world_generator, x, y)))
    return section_info;

  section_info = world_generator_section_info_add(world_generator, x, y);
  for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
    for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
    {
      int real_x = section_info->x * CHUNK_WIDTH + (int)x;
      int real_y = section_info->y * CHUNK_WIDTH + (int)y;
      section_info->heights[y][x] = world_generator_get_height(world_generator, world, real_x, real_y);
    }

  return section_info;
}


void world_generator_update(struct world_generator *world_generator, struct world *world)
{
  if(!world_generator->player_spawned)
  {
    world_generator->player_spawned     = true;
    world->player_transform.translation = vec3(0.5f, 0.5f, world_generator_get_height(world_generator, world, 0, 0) + 2.0f);
    world->player_transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
  }

  int x = floorf(world->player_transform.translation.x / CHUNK_WIDTH);
  int y = floorf(world->player_transform.translation.y / CHUNK_WIDTH);
  int z = floorf(world->player_transform.translation.z / CHUNK_WIDTH);
  for(int dz = -8; dz<=8; ++dz)
    for(int dy = -8; dy<=8; ++dy)
      for(int dx = -8; dx<=8; ++dx)
        if(!world_chunk_lookup(world, x+dx, y+dy, z+dz))
          world_generator_generate_chunk(world_generator, world, x+dx, y+dy, z+dz);
}

void world_generator_generate_chunk(struct world_generator *world_generator, struct world *world, int x, int y, int z)
{
  (void)world_generator;

  struct chunk *chunk = world_chunk_add(world, x, y, z);
  chunk->mesh_dirty = false;

  if(chunk->z < 0)
  {
    for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
      for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
        for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;

    return; // Fast-path!?
  }

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

static void vec2_rotate(struct vec2 *vec, float degree)
{
  float new_x = vec->x * +cosf(degree) + vec->y * +sinf(degree);
  float new_y = vec->x * -sinf(degree) + vec->y * +cosf(degree);
  vec->x = new_x;
  vec->y = new_y;
}

float world_generator_get_height(struct world_generator *world_generator, struct world *world, int x, int y)
{
  (void)world_generator;

  float value = 0.0f;

  seed_t      seed     = world->seed;
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
