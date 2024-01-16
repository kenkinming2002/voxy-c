#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

#include "math.h"
#include "noise.h"
#include "transform.h"
#include "window.h"

#include <stdbool.h>
#include <stddef.h>

#define CHUNK_WIDTH 16

#define TILE_ID_GRASS 0
#define TILE_ID_STONE 1
#define TILE_ID_EMPTY 2

/********
 * Tile *
 ********/
struct tile
{
  uint8_t id;
};

/*********
 * Chunk *
 *********/
struct chunk
{
  struct chunk *next;
  size_t        hash;

  int x, y, z;

  struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  bool mesh_dirty;
};

/*********
 * World *
 *********/
struct world
{
  seed_t seed;

  struct chunk **chunks;
  size_t         chunk_capacity;
  size_t         chunk_load;

  struct transform player_transform;
};

void world_init(struct world *world, seed_t seed);
void world_deinit(struct world *world);

void world_chunk_rehash(struct world *world, size_t new_capacity);
struct chunk *world_chunk_insert(struct world *world, int x, int y, int z);
struct chunk *world_chunk_lookup(struct world *world, int x, int y, int z);

void world_update(struct world *world, struct window *window);
void world_update_player_control(struct world *world, struct window *window);

/*******************
 * Chunk Adjacency *
 *******************/
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
struct tile *chunk_adjacency_tile_lookup(struct chunk_adjacency *chunk_adjacency, int cx, int cy, int cz);

#endif // VOXY_WORLD_H
