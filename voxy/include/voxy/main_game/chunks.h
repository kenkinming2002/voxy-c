#ifndef MAIN_GAME_CHUNKS_H
#define MAIN_GAME_CHUNKS_H

#include <voxy/math/vector.h>

#include <voxy/types/chunk.h>
#include <voxy/types/chunk_hash_table.h>

extern struct chunk_hash_table chunks;

struct chunk *chunk_lookup(ivec3_t position);
void chunk_insert_unchecked(struct chunk *chunk);

#define chunk_for_each(it) \
  for(size_t i=0; i<chunks.bucket_count; ++i) \
    for(struct chunk *it = chunks.buckets[i].head; it; it = it->next)

struct block *block_get(ivec3_t position);
void block_set(ivec3_t position, uint8_t block_id);

void invalidate_light_at(ivec3_t position);
void invalidate_mesh_at(ivec3_t position);

#endif // MAIN_GAME_CHUNKS_H
