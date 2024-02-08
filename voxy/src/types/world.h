#ifndef TYPES_WORLD_H
#define TYPES_WORLD_H

#include <voxy/math/random.h>

#include <types/player.h>
#include <types/chunk_hash_table.h>

struct world
{
  seed_t seed;

  struct player_entity    player;
  struct chunk_hash_table chunks;
};

void world_init(struct world *world, seed_t seed);
void world_fini(struct world *world);

struct block *world_get_block(struct world *world, ivec3_t position);
void world_invalidate_block(struct world *world, ivec3_t position);
void world_block_set_id(struct world *world, ivec3_t position, uint8_t id);

void world_chunk_insert_unchecked(struct world *world, struct chunk *chunk);

#endif // TYPES_WORLD_H
