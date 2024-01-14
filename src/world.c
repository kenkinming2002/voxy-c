#include <voxy/world.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

static float get_height(seed_t seed, int y, int x)
{
  float value = 0.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 15.0f)) * 30.0f + 30.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 7.0f))  * 15.0f + 10.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 3.0f))  * 3.0f  + 3.0f;
  return value;
}

void chunk_init(struct chunk *chunk, seed_t seed)
{
  if(chunk->z < 0)
    return; // Fast-path

  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
  for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
    for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
    {
      int real_y = chunk->y * CHUNK_WIDTH + (int)y;
      int real_x = chunk->x * CHUNK_WIDTH + (int)x;
      heights[y][x] = get_height(seed, real_y, real_x);
    }

  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        int real_z = chunk->z * CHUNK_WIDTH + (int)z;
        if(real_z <= heights[y][x])
          chunk->tiles[z][y][x].id = TILE_ID_STONE;
        else if(real_z <= heights[y][x] + 1.0f)
          chunk->tiles[z][y][x].id = TILE_ID_GRASS;
        else
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
      }

  chunk->remesh = true;
}

void world_init(struct world *world)
{
  world->chunks         = NULL;
  world->chunk_capacity = 0;
  world->chunk_load     = 0;
}

void world_deinit(struct world *world)
{
  for(size_t i=0; i<world->chunk_capacity; ++i)
  {
    struct chunk *chunk = world->chunks[i];
    while(chunk)
    {
      struct chunk *tmp = chunk;
      chunk = chunk->next;
      free(tmp);
    }
  }
  free(world->chunks);
}

static inline size_t hash(int x, int y, int z)
{
  // I honestly do not know what I am doing here
  return x * 23 + y * 31 + z * 47;
}

void world_chunk_rehash(struct world *world, size_t new_capacity)
{
  struct chunk *orphans = NULL;
  for(size_t i=0; i<world->chunk_capacity; ++i)
  {
    struct chunk **head = &world->chunks[i];
    while(*head)
      if((*head)->hash % new_capacity != i)
      {
        struct chunk *orphan = *head;
        *head = (*head)->next;

        orphan->next = orphans;
        orphans      = orphan;
      }
      else
        head = &(*head)->next;
  }

  world->chunks = realloc(world->chunks, new_capacity * sizeof *world->chunks);
  for(size_t i=world->chunk_capacity; i<new_capacity; ++i)
    world->chunks[i] = NULL;
  world->chunk_capacity = new_capacity;

  while(orphans)
  {
    struct chunk *orphan = orphans;
    orphans = orphans->next;

    orphan->next = world->chunks[orphan->hash % world->chunk_capacity];
    world->chunks[orphan->hash % world->chunk_capacity] = orphan;
  }
}

struct chunk *world_chunk_add(struct world *world, int x, int y, int z)
{
  if(world->chunk_capacity == 0)
    world_chunk_rehash(world, 32);
  else if(world->chunk_load * 4 >= world->chunk_capacity * 3)
    world_chunk_rehash(world, world->chunk_capacity * 2);

  struct chunk *chunk = malloc(sizeof *chunk);
  chunk->x = x;
  chunk->y = y;
  chunk->z = z;
  chunk->hash = hash(x, y, z);
  chunk->next = world->chunks[chunk->hash % world->chunk_capacity];
  world->chunks[chunk->hash % world->chunk_capacity] = chunk;
  world->chunk_load += 1;
  return chunk;
}

struct chunk *world_chunk_lookup(struct world *world, int x, int y, int z)
{
  if(world->chunk_capacity == 0)
    return NULL;

  size_t h = hash(x, y, z);
  for(struct chunk *chunk = world->chunks[h % world->chunk_capacity]; chunk; chunk = chunk->next)
    if(chunk->hash == h && chunk->x == x && chunk->y == y && chunk->z == z)
      return chunk;
  return NULL;
}

