#include <voxy/chunk.h>

#include <stdlib.h>

void chunk_randomize(struct chunk *chunk)
{
  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        chunk->tiles[z][y][x].present = rand() % 2 == 0;
        chunk->tiles[z][y][x].color.r = (float)rand() / (float)RAND_MAX;
        chunk->tiles[z][y][x].color.g = (float)rand() / (float)RAND_MAX;
        chunk->tiles[z][y][x].color.b = (float)rand() / (float)RAND_MAX;
      }

  chunk->remesh = true;
}

