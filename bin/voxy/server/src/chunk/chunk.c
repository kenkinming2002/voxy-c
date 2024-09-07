#include "chunk.h"

#include <voxy/protocol/chunk.h>

#include <stdatomic.h>
#include <stdlib.h>

struct chunk *chunk_create(void)
{
  struct chunk *chunk = malloc(sizeof *chunk);
  return chunk;
}

void chunk_destroy(struct chunk *chunk)
{
  free(chunk);
}

static void check_position(ivec3_t position)
{
  assert(0 <= position.x && position.x < VOXY_CHUNK_WIDTH);
  assert(0 <= position.y && position.y < VOXY_CHUNK_WIDTH);
  assert(0 <= position.z && position.z < VOXY_CHUNK_WIDTH);

}

uint8_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position)
{
  check_position(position);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  return chunk->block_ids[index];
}

uint8_t chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position)
{
  check_position(position);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const unsigned q = index / 2;
  const unsigned r = index % 2;
  return (chunk->block_light_levels[q] >> (4 * r)) & 0xF;
}

void chunk_set_block_id(struct chunk *chunk, ivec3_t position, uint8_t id)
{
  check_position(position);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  chunk->block_ids[index] = id;
}

void chunk_set_block_light_level(struct chunk *chunk, ivec3_t position, uint8_t light_level)
{
  check_position(position);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const unsigned q = index / 2;
  const unsigned r = index % 2;
  chunk->block_light_levels[q] &= ~(((1 << 4) - 1) << (r * 4));
  chunk->block_light_levels[q] |= light_level << (r * 4);
}

void chunk_get_block_light_level_atomic(struct chunk *chunk, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  check_position(position);

  const size_t offset = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);

  *tmp = atomic_load((_Atomic unsigned char *)&chunk->block_light_levels[q]);
  *light_level = (*tmp >> (r * 4)) & ((1 << 4) - 1);
}

bool chunk_set_block_light_level_atomic(struct chunk *chunk, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  check_position(position);

  const size_t offset = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);

  unsigned char desired = *tmp;
  desired &= ~(((1 << 4) - 1) << (r * 4));
  desired |= *light_level << (r * 4);
  if(!atomic_compare_exchange_strong((_Atomic unsigned char *)&chunk->block_light_levels[q], tmp, desired))
  {
    *light_level = (*tmp >> (r * 4)) & ((1 << 4) - 1);
    return false;
  }

  return true;
}

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t chunk_key(struct chunk *chunk) { return chunk->position; }
size_t chunk_hash(ivec3_t position) { return ivec3_hash(position); }
int chunk_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void chunk_dispose(struct chunk *chunk) { chunk_destroy(chunk); }

