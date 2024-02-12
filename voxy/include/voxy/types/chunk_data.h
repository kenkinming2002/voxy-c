#ifndef TYPES_CHUNK_DATA_H
#define TYPES_CHUNK_DATA_H

#include <voxy/config.h>
#include <voxy/types/block.h>

struct chunk_data
{
  struct block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
};

void chunk_data_dispose(struct chunk_data *chunk_data);

#endif // TYPES_CHUNK_DATA_H


