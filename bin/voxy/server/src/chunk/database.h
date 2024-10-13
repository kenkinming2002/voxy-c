#ifndef CHUNK_DATABASE_H
#define CHUNK_DATABASE_H

#include "chunk.h"

struct voxy_chunk *voxy_chunk_database_load(ivec3_t position);
int voxy_chunk_database_save(struct voxy_chunk *chunk);

#endif // CHUNK_DATABASE_H
