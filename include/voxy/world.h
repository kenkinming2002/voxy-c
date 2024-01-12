#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

#include <voxy/math.h>
#include <voxy/noise.h>

#include <stddef.h>
#include <stdbool.h>

#define CHUNK_WIDTH 16

/********
 * Tile *
 ********/
struct tile
{
  bool present;
  struct vec3 color;
};

/*********
 * Chunk *
 *********/
struct chunk
{
  int z;
  int y;
  int x;

  struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

  bool remesh;
};

void chunk_init(struct chunk *chunk, seed_t seed);
void chunk_randomize(struct chunk *chunk);

/*********
 * World *
 *********/
struct world
{
  // FIXME: Use a proper hash table
  struct chunk *chunks;
  size_t        chunk_count;
  size_t        chunk_capacity;
};

void world_init(struct world *world);
void world_deinit(struct world *world);

struct chunk *world_chunk_add(struct world *world, int z, int y, int x);
struct chunk *world_chunk_lookup(struct world *world, int z, int y, int x);

void world_update(struct world *world);

#endif // VOXY_WORLD_H
