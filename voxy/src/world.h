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
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
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
#define TILE_ID_ETHER 4

struct tile
{
  uint8_t id;
  uint8_t ether       : 1;
  uint8_t light_level : 4;
};

struct chunk
{
  struct chunk *next;
  size_t        hash;
  ivec3_t       position;

  struct chunk *bottom;
  struct chunk *top;
  struct chunk *back;
  struct chunk *front;
  struct chunk *left;
  struct chunk *right;

  struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  bool mesh_dirty;
  bool light_dirty;
};

struct player
{
  bool             spawned;
  struct transform transform;
};

struct world
{
  seed_t seed;

  struct player           player;
  struct chunk_hash_table chunks;
};

void world_init(struct world *world, seed_t seed);
void world_deinit(struct world *world);

void world_chunk_insert_unchecked(struct world *world, struct chunk *chunk);

void world_update(struct world *world, struct window *window, float dt);
void world_update_player_control(struct world *world, struct window *window, float dt);
void world_update_light(struct world *world);

#endif // VOXY_WORLD_H
