#include "world.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <stdlib.h>

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
  if(chunk->chunk_mesh)
  {
    glDeleteVertexArrays(1, &chunk->chunk_mesh->vao_opaque);
    glDeleteBuffers(1, &chunk->chunk_mesh->vbo_opaque);
    glDeleteBuffers(1, &chunk->chunk_mesh->ibo_opaque);
  }

  free(chunk->chunk_mesh);
  free(chunk->chunk_data);
  free(chunk);
}

void world_init(struct world *world, seed_t seed)
{
  world->seed = seed;

  chunk_hash_table_init(&world->chunks);

  world->player.spawned = false;
}

void world_fini(struct world *world)
{
  chunk_hash_table_dispose(&world->chunks);
}

struct tile *world_get_tile(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t tile_position;
  for(int i=0; i<3; ++i)
  {
    tile_position.values[i]  = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    chunk_position.values[i] = (position.values[i] - tile_position.values[i]) / CHUNK_WIDTH;
  }

  struct chunk *chunk = chunk_hash_table_lookup(&world->chunks, chunk_position);
  if(!chunk || !chunk->chunk_data)
    return NULL;

  return &chunk->chunk_data->tiles[tile_position.z][tile_position.y][tile_position.x];
}

void world_invalidate_tile(struct world *world, ivec3_t position)
{
  ivec3_t chunk_position;
  ivec3_t tile_position;
  for(int i=0; i<3; ++i)
  {
    tile_position.values[i]  = ((position.values[i] % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH;
    chunk_position.values[i] = (position.values[i] - tile_position.values[i]) / CHUNK_WIDTH;
  }

  struct chunk *chunk = chunk_hash_table_lookup(&world->chunks, chunk_position);
  if(chunk)
  {
    chunk->light_dirty = true;
    chunk->mesh_dirty = true;
  }
}

void world_tile_set_id(struct world *world, ivec3_t position, uint8_t id)
{
  struct tile *tile = world_get_tile(world, position);
  if(tile)
  {
    tile->id = id;

    world_invalidate_tile(world, position);
    world_invalidate_tile(world, ivec3_add(position, ivec3(-1, 0, 0)));
    world_invalidate_tile(world, ivec3_add(position, ivec3( 1, 0, 0)));
    world_invalidate_tile(world, ivec3_add(position, ivec3(0, -1, 0)));
    world_invalidate_tile(world, ivec3_add(position, ivec3(0,  1, 0)));
    world_invalidate_tile(world, ivec3_add(position, ivec3(0, 0, -1)));
    world_invalidate_tile(world, ivec3_add(position, ivec3(0, 0,  1)));
  }
}

void world_chunk_insert_unchecked(struct world *world, struct chunk *chunk)
{
  chunk->left   = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3(-1,  0,  0)));
  chunk->right  = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 1,  0,  0)));
  chunk->back   = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0, -1,  0)));
  chunk->front  = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  1,  0)));
  chunk->bottom = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0, -1)));
  chunk->top    = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0,  1)));

  if(chunk->left)   chunk->left->right = chunk;
  if(chunk->right)  chunk->right->left = chunk;
  if(chunk->back)   chunk->back->front = chunk;
  if(chunk->front)  chunk->front->back = chunk;
  if(chunk->bottom) chunk->bottom->top = chunk;
  if(chunk->top)    chunk->top->bottom = chunk;

  chunk_hash_table_insert_unchecked(&world->chunks, chunk);
}

