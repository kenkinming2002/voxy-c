#ifndef VOXY_SCENE_MAIN_GAME_TYPES_REGISTRY_H
#define VOXY_SCENE_MAIN_GAME_TYPES_REGISTRY_H

#include <voxy/math/vector.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

struct chunk;
struct block;
struct item;
struct entity;

typedef uint8_t block_id_t;
typedef uint8_t item_id_t;
typedef uint8_t entity_id_t;

#define BLOCK_MAX (block_id_t)(-1)
#define ITEM_MAX  (item_id_t) (-1)
#define ENTITY_MAX  (entity_id_t) (-1)

#define BLOCK_NONE (block_id_t)(-1)
#define ITEM_NONE  (item_id_t) (-1)
#define ENTITY_NONE (entity_id_t) (-1)

enum block_type
{
  BLOCK_TYPE_INVISIBLE,
  BLOCK_TYPE_TRANSPARENT,
  BLOCK_TYPE_OPAQUE,
};

enum block_face
{
  BLOCK_FACE_LEFT   = 0,
  BLOCK_FACE_RIGHT  = 1,
  BLOCK_FACE_BACK   = 2,
  BLOCK_FACE_FRONT  = 3,
  BLOCK_FACE_BOTTOM = 4,
  BLOCK_FACE_TOP    = 5,
  BLOCK_FACE_COUNT
};

struct block_info
{
  const char *mod;
  const char *name;

  enum block_type type;

  uint8_t ether       : 1;
  uint8_t light_level : 4;

  const char *textures[BLOCK_FACE_COUNT];

  void(*on_create)(struct entity *entity, struct chunk *chunk, struct block *block);
  void(*on_destroy)(struct entity *entity, struct chunk *chunk, struct block *block);
};

struct item_info
{
  const char *mod;
  const char *name;

  const char *texture;

  void(*on_use)(struct entity *entity, struct item *item);
};

struct entity_info
{
  const char *mod;
  const char *name;

  const char *mesh;
  const char *texture;

  fvec3_t hitbox_offset;
  fvec3_t hitbox_dimension;

  void(*on_dispose)(struct entity *entity);

  void(*on_update)(struct entity *entity, float dt);

  bool(*on_save)(const struct entity *entity, FILE *file);
  bool(*on_load)(struct entity *entity, FILE *file);
};

const char *block_type_as_str(enum block_type block_type);

block_id_t register_block_info(struct block_info block_info);
item_id_t  register_item_info(struct item_info item_info);
entity_id_t register_entity_info(struct entity_info entity_info);

const struct block_info *query_block_info(block_id_t block_id);
const struct item_info *query_item_info(item_id_t item_id);
const struct entity_info *query_entity_info(entity_id_t entity_id);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_REGISTRY_H
