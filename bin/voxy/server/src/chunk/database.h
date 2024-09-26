#ifndef CHUNK_DATABASE_H
#define CHUNK_DATABASE_H

#include "chunk.h"

struct chunk *chunk_database_load(ivec3_t position);
int chunk_database_save(struct chunk *chunk);

#endif // CHUNK_DATABASE_H
