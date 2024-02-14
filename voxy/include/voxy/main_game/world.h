#ifndef MAIN_GAME_WORLD_H
#define MAIN_GAME_WORLD_H

#include <voxy/math/vector.h>

#include <voxy/types/chunk.h>
#include <voxy/types/chunk_hash_table.h>

extern struct chunk_hash_table chunks;

extern struct chunk *chunks_invalidated_light_head;
extern struct chunk *chunks_invalidated_light_tail;

extern struct chunk *chunks_invalidated_mesh_head;
extern struct chunk *chunks_invalidated_mesh_tail;

#define world_chunk_for_each(it) \
  for(size_t i=0; i<chunks.bucket_count; ++i) \
    for(struct chunk *it = chunks.buckets[i].head; it; it = it->next)

struct chunk *world_chunk_lookup(ivec3_t position);
struct chunk *world_chunk_create(ivec3_t position);
void world_chunk_invalidate_light(struct chunk *chunk);
void world_chunk_invalidate_mesh(struct chunk *chunk);

struct block *world_block_get(ivec3_t position);
void world_block_set(ivec3_t position, uint8_t block_id);
void world_block_invalidate_light(ivec3_t position);
void world_block_invalidate_mesh(ivec3_t position);

#endif // MAIN_GAME_WORLD_H
