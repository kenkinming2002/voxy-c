#ifndef VOXY_WORLD_H
#define VOXY_WORLD_H

#include "entity.h"
#include "config.h"
#include "glad/glad.h"
#include "transform.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#include <voxy/config.h>

#include <stdbool.h>
#include <stddef.h>

#define BLOCK_NONE UINT8_MAX
#define ITEM_NONE  UINT8_MAX

struct block
{
  uint8_t id;
  uint8_t ether       : 1;
  uint8_t light_level : 4;
};

struct chunk_data
{
  struct block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
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
#define ITEM_MAX_STACK 64

struct item
{
  uint8_t id;
  uint8_t count;
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

struct player_entity
{
  struct entity base;

  bool spawned;
  bool third_person;

  struct inventory inventory;
  struct hotbar    hotbar;
  struct item     *item_hovered;
  struct item      item_held;
  fvec2_t          item_held_position;

  float cooldown;

  bool has_target_destroy;
  bool has_target_place;

  ivec3_t target_destroy;
  ivec3_t target_place;
};

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

#endif // VOXY_WORLD_H
