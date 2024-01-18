#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

#include "math.h"
#include "noise.h"
#include "transform.h"
#include "window.h"
#include "config.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE struct ivec3
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#include <stdbool.h>
#include <stddef.h>

#define TILE_ID_GRASS 0
#define TILE_ID_STONE 1
#define TILE_ID_WATER 2
#define TILE_ID_EMPTY 3

struct tile
{
  uint8_t id;
};

struct chunk
{
  struct chunk *next;
  size_t        hash;

  struct ivec3 position;

  struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  bool mesh_dirty;
};

struct world
{
  seed_t seed;

  struct transform        player_transform;
  struct chunk_hash_table chunks;
};

void world_init(struct world *world, seed_t seed);
void world_deinit(struct world *world);

void world_update(struct world *world, struct window *window);
void world_update_player_control(struct world *world, struct window *window);

struct chunk_adjacency
{
  struct chunk *chunk;

  struct chunk *bottom;
  struct chunk *top;

  struct chunk *back;
  struct chunk *front;

  struct chunk *left;
  struct chunk *right;
};

void chunk_adjacency_init(struct chunk_adjacency *chunk_adjacency, struct world *world, struct chunk *chunk);
struct tile *chunk_adjacency_tile_lookup(struct chunk_adjacency *chunk_adjacency, struct ivec3 cposition);

#endif // VOXY_WORLD_H
