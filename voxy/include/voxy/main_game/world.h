#ifndef MAIN_GAME_WORLD_H
#define MAIN_GAME_WORLD_H

#include <voxy/math/vector.h>

#include <voxy/types/chunk.h>
#include <voxy/types/chunk_hash_table.h>

extern struct chunk_hash_table chunks;

struct chunk *world_chunk_lookup(ivec3_t position);
void world_chunk_insert_unchecked(struct chunk *chunk);

#define world_chunk_for_each(it) \
  for(size_t i=0; i<chunks.bucket_count; ++i) \
    for(struct chunk *it = chunks.buckets[i].head; it; it = it->next)

struct block *world_get_block(ivec3_t position);
void world_set_block(ivec3_t position, uint8_t block_id);

void world_invalidate_light_at(ivec3_t position);
void world_invalidate_mesh_at(ivec3_t position);

#endif // MAIN_GAME_WORLD_H
