#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

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

struct chunk *world_chunk_add(struct world *world, int z, int y, int x);
struct chunk *world_chunk_lookup(struct world *world, int z, int y, int x);

void world_update(struct world *world);

#endif // VOXY_WORLD_H
