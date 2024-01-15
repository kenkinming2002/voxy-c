#include "world.h"

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

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

void world_chunk_remesh_insert(struct world *world, struct chunk *chunk)
{
  if(chunk->remesh)
    return;

  chunk->remesh            = true;
  chunk->remesh_next       = world->chunk_remesh_list;
  world->chunk_remesh_list = chunk;
}

void world_init(struct world *world, seed_t seed)
{
  world->seed              = seed;

  world->chunks            = NULL;
  world->chunk_capacity    = 0;
  world->chunk_load        = 0;
  world->chunk_remesh_list = NULL;

  world->player_transform.translation = vec3(10.0f, -10.0f, 40.0f);
  world->player_transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
}

void world_update(struct world *world, struct window *window)
{
  world_update_player_control(world, window);
  world_update_chunk_generate(world);
}

void world_update_player_control(struct world *world, struct window *window)
{
  static const float MOVE_SPEED = 1.0f;
  static const float PAN_SPEED  = 0.001f;

  struct vec3 rotation = vec3_zero();
  struct vec3 translation = vec3_zero();

  window_get_mouse_motion(window, &rotation.yaw, &rotation.pitch);
  window_get_keyboard_motion(window, &translation.x, &translation.y, &translation.z);

  rotation    = vec3_mul_s(rotation, PAN_SPEED);
  translation = vec3_normalize(translation);
  translation = vec3_mul_s(translation, MOVE_SPEED);

  transform_rotate(&world->player_transform, rotation);
  transform_local_translate(&world->player_transform, translation);
}

void world_update_chunk_generate(struct world *world)
{
  int x = floorf(world->player_transform.translation.x / CHUNK_WIDTH);
  int y = floorf(world->player_transform.translation.y / CHUNK_WIDTH);
  int z = floorf(world->player_transform.translation.z / CHUNK_WIDTH);
  for(int dz = -4; dz<=4; ++dz)
    for(int dy = -4; dy<=4; ++dy)
      for(int dx = -4; dx<=4; ++dx)
        if(!world_chunk_lookup(world, x+dx, y+dy, z+dz))
          world_chunk_generate(world, x+dx, y+dy, z+dz);
}

static float get_height(seed_t seed, int y, int x)
{
  float value = 0.0f;

  value += perlin2(seed, vec2_div_s(vec2(x, y), 200.0f)) * 140.0f + 140.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 100.0f)) * 70.0f  + 70.0f;

  value += perlin2(seed, vec2_div_s(vec2(x, y), 20.0f))  * 10.0f + 10.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 5.0f))   * 5.0f  + 3.0f;

  return value;
}

void world_chunk_generate(struct world *world, int x, int y, int z)
{
  struct chunk *chunk = world_chunk_add(world, x, y, z);
  chunk->remesh      = false;
  chunk->remesh_next = NULL;

  if(chunk->z < 0)
  {
    for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
      for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
        for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;

    return; // Fast-path!?
  }

  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
  for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
    for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
    {
      int real_y = chunk->y * CHUNK_WIDTH + (int)y;
      int real_x = chunk->x * CHUNK_WIDTH + (int)x;
      heights[y][x] = get_height(world->seed, real_y, real_x);
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

  struct chunk_adjacency chunk_adjacency;
  chunk_adjacency_init(&chunk_adjacency, world, chunk);
  if(chunk_adjacency.chunk)  world_chunk_remesh_insert(world, chunk_adjacency.chunk);
  if(chunk_adjacency.bottom) world_chunk_remesh_insert(world, chunk_adjacency.bottom);
  if(chunk_adjacency.top)    world_chunk_remesh_insert(world, chunk_adjacency.top);
  if(chunk_adjacency.back)   world_chunk_remesh_insert(world, chunk_adjacency.back);
  if(chunk_adjacency.front)  world_chunk_remesh_insert(world, chunk_adjacency.front);
  if(chunk_adjacency.left)   world_chunk_remesh_insert(world, chunk_adjacency.left);
  if(chunk_adjacency.right)  world_chunk_remesh_insert(world, chunk_adjacency.right);
}

/*******************
 * Chunk Adjacency *
 *******************/
void chunk_adjacency_init(struct chunk_adjacency *chunk_adjacency, struct world *world, struct chunk *chunk)
{
  chunk_adjacency->chunk = chunk;
  chunk_adjacency->left   = world_chunk_lookup(world, chunk->x-1, chunk->y, chunk->z);
  chunk_adjacency->right  = world_chunk_lookup(world, chunk->x+1, chunk->y, chunk->z);
  chunk_adjacency->back   = world_chunk_lookup(world, chunk->x, chunk->y-1, chunk->z);
  chunk_adjacency->front  = world_chunk_lookup(world, chunk->x, chunk->y+1, chunk->z);
  chunk_adjacency->bottom = world_chunk_lookup(world, chunk->x, chunk->y, chunk->z-1);
  chunk_adjacency->top    = world_chunk_lookup(world, chunk->x, chunk->y, chunk->z+1);
}

struct tile *chunk_adjacency_tile_lookup(struct chunk_adjacency *chunk_adjacency, int x, int y, int z)
{
  if(z >= 0 && z < CHUNK_WIDTH)
    if(y >= 0 && y < CHUNK_WIDTH)
      if(x >= 0 && x < CHUNK_WIDTH)
        return &chunk_adjacency->chunk->tiles[z][y][x];

  if(z == -1)          return chunk_adjacency->bottom ? &chunk_adjacency->bottom->tiles[CHUNK_WIDTH-1][y][x] : NULL;
  if(z == CHUNK_WIDTH) return chunk_adjacency->top    ? &chunk_adjacency->top   ->tiles[0]            [y][x] : NULL;

  if(y == -1)          return chunk_adjacency->back  ? &chunk_adjacency->back ->tiles[z][CHUNK_WIDTH-1][x] : NULL;
  if(y == CHUNK_WIDTH) return chunk_adjacency->front ? &chunk_adjacency->front->tiles[z][0]            [x] : NULL;

  if(x == -1)          return chunk_adjacency->left  ? &chunk_adjacency->left ->tiles[z][y][CHUNK_WIDTH-1] : NULL;
  if(x == CHUNK_WIDTH) return chunk_adjacency->right ? &chunk_adjacency->right->tiles[z][y][0]             : NULL;

  assert(0 && "Unreachable");
}

