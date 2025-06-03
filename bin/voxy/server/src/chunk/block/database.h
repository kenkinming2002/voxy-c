#ifndef CHUNK_BLOCK_DATABASE_H
#define CHUNK_BLOCK_DATABASE_H

#include "future.h"
#include "group.h"

#include <libmath/vector.h>

void voxy_block_database_init(const char *world_directory);

struct block_group_future voxy_block_database_load(ivec3_t position);
struct unit_future voxy_block_database_save(ivec3_t position, struct voxy_block_group *block_group);

void voxy_block_database_update(void);

#endif // CHUNK_BLOCK_DATABASE_H
