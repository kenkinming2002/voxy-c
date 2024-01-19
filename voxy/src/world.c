#include "world.h"
#include "hash.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE struct ivec3
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

struct ivec3 chunk_key(struct chunk *chunk)
{
  return chunk->position;
}

size_t chunk_hash(struct ivec3 position)
{
  return hash3(position.x, position.y, position.z);
}

int chunk_compare(struct ivec3 position1, struct ivec3 position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_dispose(struct chunk *chunk)
{
  free(chunk);
}

void world_init(struct world *world, seed_t seed)
{
  world->seed = seed;

  chunk_hash_table_init(&world->chunks);

  world->player_transform.translation = vec3_zero();
  world->player_transform.rotation    = vec3_zero();
}

void world_deinit(struct world *world)
{
  chunk_hash_table_dispose(&world->chunks);
}

void world_update(struct world *world, struct window *window, float dt)
{
  world_update_player_control(world, window, dt);
}

void world_update_player_control(struct world *world, struct window *window, float dt)
{
  static const float MOVE_SPEED = 50.0f;
  static const float PAN_SPEED  = 0.2;

  struct vec3 rotation = vec3_zero();
  struct vec3 translation = vec3_zero();

  window_get_mouse_motion(window, &rotation.yaw, &rotation.pitch);
  window_get_keyboard_motion(window, &translation.x, &translation.y, &translation.z);

  rotation    = vec3_mul_s(rotation, PAN_SPEED * dt);
  translation = vec3_normalize(translation);
  translation = vec3_mul_s(translation, MOVE_SPEED * dt);

  transform_rotate(&world->player_transform, rotation);
  transform_local_translate(&world->player_transform, translation);
}

/*******************
 * Chunk Adjacency *
 *******************/
void chunk_adjacency_init(struct chunk_adjacency *chunk_adjacency, struct world *world, struct chunk *chunk)
{
  chunk_adjacency->chunk = chunk;
  chunk_adjacency->left   = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3(-1,  0,  0)));
  chunk_adjacency->right  = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 1,  0,  0)));
  chunk_adjacency->back   = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0, -1,  0)));
  chunk_adjacency->front  = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  1,  0)));
  chunk_adjacency->bottom = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0, -1)));
  chunk_adjacency->top    = chunk_hash_table_lookup(&world->chunks, ivec3_add(chunk->position, ivec3( 0,  0,  1)));
}

struct tile *chunk_adjacency_tile_lookup(struct chunk_adjacency *chunk_adjacency, struct ivec3 cposition)
{
  if(cposition.z >= 0 && cposition.z < CHUNK_WIDTH)
    if(cposition.y >= 0 && cposition.y < CHUNK_WIDTH)
      if(cposition.x >= 0 && cposition.x < CHUNK_WIDTH)
        return &chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x];

  if(cposition.z == -1)          return chunk_adjacency->bottom ? &chunk_adjacency->bottom->tiles[CHUNK_WIDTH-1][cposition.y][cposition.x] : NULL;
  if(cposition.z == CHUNK_WIDTH) return chunk_adjacency->top    ? &chunk_adjacency->top   ->tiles[0]            [cposition.y][cposition.x] : NULL;

  if(cposition.y == -1)          return chunk_adjacency->back  ? &chunk_adjacency->back ->tiles[cposition.z][CHUNK_WIDTH-1][cposition.x] : NULL;
  if(cposition.y == CHUNK_WIDTH) return chunk_adjacency->front ? &chunk_adjacency->front->tiles[cposition.z][0]            [cposition.x] : NULL;

  if(cposition.x == -1)          return chunk_adjacency->left  ? &chunk_adjacency->left ->tiles[cposition.z][cposition.y][CHUNK_WIDTH-1] : NULL;
  if(cposition.x == CHUNK_WIDTH) return chunk_adjacency->right ? &chunk_adjacency->right->tiles[cposition.z][cposition.y][0]             : NULL;

  assert(0 && "Unreachable");
}

