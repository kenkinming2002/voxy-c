#ifndef TYPES_WORLD_H
#define TYPES_WORLD_H

#include <voxy/math/random.h>

#include <voxy/types/block.h>
#include <voxy/types/player.h>
#include <voxy/types/chunk_hash_table.h>

struct world
{
  seed_t seed;

  bool                    player_spawned;
  struct player_entity    player;

  struct chunk_hash_table chunks;
};

struct chunk *world_chunk_lookup(struct world *world, ivec3_t position);
void world_chunk_insert_unchecked(struct world *world, struct chunk *chunk);

struct block *world_get_block(struct world *world, ivec3_t position);
void world_set_block(struct world *world, ivec3_t position, uint8_t block_id);

void world_invalidate_light(struct world *world, ivec3_t position);
void world_invalidate_mesh(struct world *world, ivec3_t position);

void world_destroy_block(struct world *world, ivec3_t position);
void world_place_block(struct world *world, ivec3_t position, uint8_t block_id);

#endif // TYPES_WORLD_H
