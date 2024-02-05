#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

#include "config.h"
#include "glad/glad.h"
#include "input.h"
#include "math.h"
#include "transform.h"
#include "world_generator.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#include <stdbool.h>
#include <stddef.h>

#define BLOCK_NONE UINT8_MAX
#define ITEM_NONE  UINT8_MAX

struct tile
{
  uint8_t id;
  uint8_t ether       : 1;
  uint8_t light_level : 4;
};

struct chunk_data
{
  struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
};

struct chunk_mesh
{
  GLuint vao_opaque;
  GLuint vbo_opaque;
  GLuint ibo_opaque;
  GLsizei count_opaque;

  GLuint vao_transparent;
  GLuint vbo_transparent;
  GLuint ibo_transparent;
  GLsizei count_transparent;
};

struct chunk
{
  struct chunk *next;
  size_t        hash;
  ivec3_t       position;

  struct chunk *bottom;
  struct chunk *top;
  struct chunk *back;
  struct chunk *front;
  struct chunk *left;
  struct chunk *right;

  struct chunk_data *chunk_data;
  struct chunk_mesh *chunk_mesh;

  bool mesh_dirty;
  bool light_dirty;
};

#define HOTBAR_SIZE 9
#define INVENTORY_SIZE_HORIZONTAL 9
#define INVENTORY_SIZE_VERTICAL   5

struct item
{
  uint8_t id;
};

struct inventory
{
  struct item items[INVENTORY_SIZE_VERTICAL][INVENTORY_SIZE_HORIZONTAL];
  bool opened;
};

struct hotbar
{
  struct item items[HOTBAR_SIZE];
  uint8_t selection;
};

struct player
{
  bool spawned;

  struct transform transform;

  struct inventory inventory;
  struct hotbar    hotbar;
  struct item     *hover;

  float cooldown;

  bool has_target_destroy;
  bool has_target_place;

  ivec3_t target_destroy;
  ivec3_t target_place;
};

struct world
{
  seed_t seed;

  struct player           player;
  struct chunk_hash_table chunks;
};

void world_init(struct world *world, seed_t seed);
void world_fini(struct world *world);

struct tile *world_get_tile(struct world *world, ivec3_t position);
void world_invalidate_tile(struct world *world, ivec3_t position);
void world_tile_set_id(struct world *world, ivec3_t position, uint8_t id);

void world_chunk_insert_unchecked(struct world *world, struct chunk *chunk);

void world_update(struct world *world, struct world_generator *world_generator, struct resource_pack *resource_pack, struct input *input, float dt);

void world_update_player_control(struct world *world, struct resource_pack *resource_pack, struct input *input, float dt);
void world_update_generate(struct world *world, struct world_generator *world_generator, struct resource_pack *resource_pack);
void world_update_light(struct world *world, struct resource_pack *resource_pack);
void world_update_mesh(struct world *world, struct resource_pack *resource_pack);

#endif // VOXY_WORLD_H
