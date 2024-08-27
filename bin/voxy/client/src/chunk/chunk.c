#include "chunk.h"

#include <voxy/protocol/chunk.h>

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

uint8_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position)
{
  assert(0 <= position.x && position.x < VOXY_CHUNK_WIDTH);
  assert(0 <= position.y && position.y < VOXY_CHUNK_WIDTH);
  assert(0 <= position.z && position.z < VOXY_CHUNK_WIDTH);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  return chunk->block_ids[index];
}

uint8_t chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position)
{
  assert(0 <= position.x && position.x < VOXY_CHUNK_WIDTH);
  assert(0 <= position.y && position.y < VOXY_CHUNK_WIDTH);
  assert(0 <= position.z && position.z < VOXY_CHUNK_WIDTH);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const unsigned q = index / 2;
  const unsigned r = index % 2;
  return chunk->block_light_levels[q] >> (4 * r) & 0xF;
}

static void traverse(const struct chunk **chunk, ivec3_t *position)
{
  while(*chunk && position->x < 0) { (*chunk) = (*chunk)->neighbours[DIRECTION_LEFT];   position->x += VOXY_CHUNK_WIDTH; }
  while(*chunk && position->y < 0) { (*chunk) = (*chunk)->neighbours[DIRECTION_BACK];   position->y += VOXY_CHUNK_WIDTH; }
  while(*chunk && position->z < 0) { (*chunk) = (*chunk)->neighbours[DIRECTION_BOTTOM]; position->z += VOXY_CHUNK_WIDTH; }

  while(*chunk && position->x >= VOXY_CHUNK_WIDTH) { (*chunk) = (*chunk)->neighbours[DIRECTION_RIGHT]; position->x -= VOXY_CHUNK_WIDTH; }
  while(*chunk && position->y >= VOXY_CHUNK_WIDTH) { (*chunk) = (*chunk)->neighbours[DIRECTION_FRONT]; position->y -= VOXY_CHUNK_WIDTH; }
  while(*chunk && position->z >= VOXY_CHUNK_WIDTH) { (*chunk) = (*chunk)->neighbours[DIRECTION_TOP];   position->z -= VOXY_CHUNK_WIDTH; }
}

uint8_t chunk_get_block_id_ex(const struct chunk *chunk, ivec3_t position, uint8_t def)
{
  traverse(&chunk, &position);
  return chunk ? chunk_get_block_id(chunk, position) : def;
}

uint8_t chunk_get_block_light_level_ex(const struct chunk *chunk, ivec3_t position, uint8_t def)
{
  traverse(&chunk, &position);
  return chunk ? chunk_get_block_light_level(chunk, position) : def;
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

