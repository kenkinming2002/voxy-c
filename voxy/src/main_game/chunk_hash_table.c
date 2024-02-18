#include <voxy/main_game/chunk_hash_table.h>

#include <voxy/main_game/chunk.h>

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t chunk_key(struct chunk *chunk)
{
  return chunk->position;
}

size_t chunk_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int chunk_compare(ivec3_t position1, ivec3_t position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_dispose(struct chunk *chunk)
{
  glDeleteVertexArrays(1, &chunk->vao_opaque);
  glDeleteBuffers(1, &chunk->vbo_opaque);
  glDeleteBuffers(1, &chunk->ibo_opaque);

  glDeleteVertexArrays(1, &chunk->vao_transparent);
  glDeleteBuffers(1, &chunk->vbo_transparent);
  glDeleteBuffers(1, &chunk->ibo_transparent);

  free(chunk);
}

