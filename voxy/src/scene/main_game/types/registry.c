#include <voxy/scene/main_game/types/registry.h>

#include <libcommon/core/log.h>

#include <assert.h>
#include <stdlib.h>

static struct block_info block_infos[BLOCK_MAX];
static struct item_info  item_infos[ITEM_MAX];
static struct entity_info entity_infos[ENTITY_MAX];

const char *block_type_as_str(enum block_type block_type)
{
  switch(block_type)
  {
  case BLOCK_TYPE_INVISIBLE:   return "invisible";
  case BLOCK_TYPE_TRANSPARENT: return "transparent";
  case BLOCK_TYPE_OPAQUE:      return "opaque";
  default:                     return "unknown";
  }
}

block_id_t register_block_info(struct block_info block_info)
{
  for(int i=0; i<BLOCK_MAX; ++i)
    if(!block_infos[i].mod  && !block_infos[i].name)
    {
      LOG_INFO("Registered block: id = %d:", i);
      LOG_INFO("  mod            = %s", block_info.mod);
      LOG_INFO("  name           = %s", block_info.name);
      LOG_INFO("  type           = %s", block_type_as_str(block_info.type));
      LOG_INFO("  light level    = %u", block_info.light_level);
      if(block_info.type != BLOCK_TYPE_INVISIBLE)
      {
        LOG_INFO("  texture left   = %s", block_info.textures[DIRECTION_LEFT]);
        LOG_INFO("  texture right  = %s", block_info.textures[DIRECTION_RIGHT]);
        LOG_INFO("  texture back   = %s", block_info.textures[DIRECTION_BACK]);
        LOG_INFO("  texture front  = %s", block_info.textures[DIRECTION_FRONT]);
        LOG_INFO("  texture bottom = %s", block_info.textures[DIRECTION_BOTTOM]);
        LOG_INFO("  texture top    = %s", block_info.textures[DIRECTION_TOP]);
      }
      block_infos[i] = block_info;
      return i;
    }

  LOG_ERROR("Failed to allocate block id");
  exit(EXIT_FAILURE);
}

item_id_t register_item_info(struct item_info item_info)
{
  for(int i=0; i<BLOCK_MAX; ++i)
    if(!item_infos[i].mod  && !item_infos[i].name)
    {
      LOG_INFO("Registered item: id = %d:", i);
      LOG_INFO("  mod     = %s", item_info.mod);
      LOG_INFO("  name    = %s", item_info.name);
      LOG_INFO("  texture = %s", item_info.texture);
      LOG_INFO("  on use  = %p", item_info.on_use);

      item_infos[i] = item_info;
      return i;
    }

  LOG_ERROR("Failed to allocate item id");
  exit(EXIT_FAILURE);
}

entity_id_t register_entity_info(struct entity_info entity_info)
{
  for(int i=0; i<ENTITY_MAX; ++i)
    if(!entity_infos[i].mod  && !entity_infos[i].name)
    {
      LOG_INFO("Registered entity: id = %d:", i);
      LOG_INFO("  mod              = %s", entity_info.mod);
      LOG_INFO("  name             = %s", entity_info.name);
      LOG_INFO("  hitbox offset    = %f %f %f", entity_info.hitbox_offset.x, entity_info.hitbox_offset.y, entity_info.hitbox_offset.z);
      LOG_INFO("  hitbox dimension = %f %f %f", entity_info.hitbox_dimension.x, entity_info.hitbox_dimension.y, entity_info.hitbox_dimension.z);

      entity_infos[i] = entity_info;
      return i;
    }

  LOG_ERROR("Failed to allocate entity id");
  exit(EXIT_FAILURE);
}

const struct block_info *query_block_info(block_id_t block_id)
{
  assert(block_id != BLOCK_NONE);
  return &block_infos[block_id];
}

const struct item_info *query_item_info(item_id_t item_id)
{
  assert(item_id != ITEM_NONE);
  return &item_infos[item_id];
}

const struct entity_info *query_entity_info(entity_id_t entity_id)
{
  assert(entity_id != ENTITY_NONE);
  return &entity_infos[entity_id];
}
