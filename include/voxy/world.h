#ifndef WORLD_H
#define WORLD_H

#include <voxy/chunk.h>

#include <stddef.h>

struct world
{
  // FIXME: Use a proper hash table
  struct chunk *chunks;
  size_t        chunk_count;
  size_t        chunk_capacity;
};

void world_init(struct world *world);

void          world_chunk_add   (struct world *world, struct chunk chunk);
struct chunk *world_chunk_lookup(struct world *world, int z, int y, int x);

void world_update(struct world *world);

#endif // WORLD_H
