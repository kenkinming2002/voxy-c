#ifndef VOXY_SCENE_MAIN_GAME_TYPES_REGISTRY_H
#define VOXY_SCENE_MAIN_GAME_TYPES_REGISTRY_H

#include <libcommon/math/vector.h>
#include <libcommon/math/direction.h>
#include <libcommon/graphics/camera.h>

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

/// Types of block with respect to the light system.
///
/// Passable means that light can passes through while opaque means that light
/// cannot passes through.
enum block_light_type
{
  BLOCK_LIGHT_TYPE_PASSABLE,
  BLOCK_LIGHT_TYPE_OPAQUE,
};

/// Types of block with respect to the rendering system.
///
/// Invisible blocks are skipped during rendering.
enum block_render_type
{
  BLOCK_RENDER_TYPE_INVISIBLE,
  BLOCK_RENDER_TYPE_TRANSPARENT,
  BLOCK_RENDER_TYPE_OPAQUE,
};

/// Types of block with respect to the physics system.
///
/// Invisible blocks have no colliders.
enum block_physics_type
{
  BLOCK_PHYSICS_TYPE_INVISIBLE,
  BLOCK_PHYSICS_TYPE_CUBE,
};

struct block_info
{
  const char *mod;
  const char *name;

  enum block_light_type light_type;
  enum block_render_type render_type;
  enum block_physics_type physics_type;

  uint8_t light_level : 4;

  const char *textures[DIRECTION_COUNT];

  void(*on_create)(struct entity *entity, struct chunk *chunk, ivec3_t position);
  void(*on_destroy)(struct entity *entity, struct chunk *chunk, ivec3_t position);
};

struct item_info
{
  const char *mod;
  const char *name;

  const char *texture;

  bool(*on_use)(struct entity *entity, struct item *item);
};

struct entity_info
{
  const char *mod;
  const char *name;

  fvec3_t hitbox_offset;
  fvec3_t hitbox_dimension;

  void(*on_dispose)(struct entity *entity);

  void(*on_update)(struct entity *entity, float dt);
  void(*on_render)(const struct entity *entity, const struct camera *camera);

  bool(*on_save)(const struct entity *entity, FILE *file);
  bool(*on_load)(struct entity *entity, FILE *file);
};

const char *block_light_type_as_str(enum block_light_type block_type);
const char *block_render_type_as_str(enum block_render_type block_type);
const char *block_physics_type_as_str(enum block_physics_type block_type);

block_id_t register_block_info(struct block_info block_info);
item_id_t  register_item_info(struct item_info item_info);
entity_id_t register_entity_info(struct entity_info entity_info);

const struct block_info *query_block_info(block_id_t block_id);
const struct item_info *query_item_info(item_id_t item_id);
const struct entity_info *query_entity_info(entity_id_t entity_id);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_REGISTRY_H
