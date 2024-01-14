#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

#include <voxy/math.h>
#include <voxy/noise.h>

#include <stddef.h>
#include <stdbool.h>

#define CHUNK_WIDTH 16

#define TILE_ID_EMPTY 0
#define TILE_ID_GRASS 1
#define TILE_ID_STONE 2

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
  bool remesh;
};

void chunk_init(struct chunk *chunk, seed_t seed);

/*********
 * World *
 *********/
struct world
{
  struct chunk **chunks;
  size_t         chunk_capacity;
  size_t         chunk_load;
};

void world_init(struct world *world);
void world_deinit(struct world *world);

void world_chunk_rehash(struct world *world, size_t new_capacity);
struct chunk *world_chunk_add(struct world *world, int x, int y, int z);
struct chunk *world_chunk_lookup(struct world *world, int x, int y, int z);

void world_update(struct world *world);

#endif // VOXY_WORLD_H
