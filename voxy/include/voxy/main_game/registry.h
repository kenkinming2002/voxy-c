#ifndef VOXY_MAIN_GAME_REGISTRY_H
#define VOXY_MAIN_GAME_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t block_id_t;
typedef uint8_t item_id_t;

#define BLOCK_MAX (block_id_t)(-1)
#define ITEM_MAX  (item_id_t) (-1)

#define BLOCK_NONE (block_id_t)(-1)
#define ITEM_NONE  (item_id_t) (-1)

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
};

struct item_info
{
  const char *mod;
  const char *name;

  const char *texture;

  void(*on_use)(uint8_t item_id);
};

const char *block_type_as_str(enum block_type block_type);

block_id_t register_block_info(struct block_info block_info);
item_id_t  register_item_info(struct item_info item_info);

const struct block_info *query_block_info(block_id_t block_id);
const struct item_info *query_item_info(item_id_t item_id);

#endif // VOXY_MAIN_GAME_REGISTRY_H
