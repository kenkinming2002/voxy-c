#include "chest.h"
#include "mod.h"

#include "block/block.h"
#include "item/chest/chest.h"
#include "entity/player/player.h"

#include <voxy/scene/main_game/types/item.h>

#include <libcommon/core/log.h>
#include <libcommon/utils/utils.h>

struct chest_block_data
{
  struct item items[5][5];
};

static block_id_t chest_block_id;

void chest_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "chest";

  block_info.light_type = BLOCK_LIGHT_TYPE_OPAQUE;
  block_info.render_type = BLOCK_RENDER_TYPE_OPAQUE;
  block_info.physics_type = BLOCK_PHYSICS_TYPE_CUBE;

  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "mod/assets/textures/chest_other.png";
  block_info.textures[DIRECTION_RIGHT]  = "mod/assets/textures/chest_other.png";
  block_info.textures[DIRECTION_BACK]   = "mod/assets/textures/chest_other.png";
  block_info.textures[DIRECTION_FRONT]  = "mod/assets/textures/chest_front.png";
  block_info.textures[DIRECTION_BOTTOM] = "mod/assets/textures/chest_other.png";
  block_info.textures[DIRECTION_TOP]    = "mod/assets/textures/chest_other.png";

  block_info.on_create = chest_block_on_create;
  block_info.on_destroy = chest_block_on_destroy;

  block_info.on_use = chest_block_on_use;

  block_info.serialize = chest_block_serialize;
  block_info.deserialize = chest_block_deserialize;

  chest_block_id = register_block_info(block_info);
}

block_id_t chest_block_id_get(void)
{
  return chest_block_id;
}

void chest_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;

  struct chest_block_data *data = malloc(sizeof *data);
  for(unsigned y=0; y<5; ++y)
    for(unsigned x=0; x<5; ++x)
    {
      data->items[y][x].id = ITEM_NONE;
      data->items[y][x].count = 0;
    }

  chunk_add_block_data(chunk, position, data);
}

void chest_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;

  struct chest_block_data *data = chunk_del_block_data(chunk, position);
  block_on_destroy_spawn_item(chunk, position, chest_item_id_get());
  free(data);
}

bool chest_block_on_use(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  struct chest_block_data *data = chunk_get_block_data(chunk, position);
  if(entity->id != player_entity_id_get())
    return false;

  struct player_opaque *player_opaque = entity->opaque;
  for(unsigned y=0; y<5; ++y)
    for(unsigned x=0; x<5; ++x)
      SWAP(player_opaque->inventory[y][x], data->items[y][x]);

  return true;
}

int chest_block_serialize(const void *_data, struct serializer *serializer)
{
  const struct chest_block_data *data = _data;
  SERIALIZE(serializer, *data);
  return 0;
}

int chest_block_deserialize(void **_data, struct deserializer *deserializer)
{
  struct chest_block_data data;
  DESERIALIZE(deserializer, data);

  struct chest_block_data *tmp = malloc(sizeof *tmp);
  *tmp = data;
  *_data = tmp;
  return 0;
}

