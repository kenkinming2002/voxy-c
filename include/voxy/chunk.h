#ifndef VOXY_CHUNK_H
#define VOXY_CHUNK_H

#include <voxy/tile.h>

#define CHUNK_WIDTH 16

struct chunk
{
  int z;
  int y;
  int x;

  struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

  bool remesh;
};

void chunk_randomize(struct chunk *chunk);

#endif // VOXY_CHUNK_H
