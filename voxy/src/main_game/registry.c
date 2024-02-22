#include <voxy/main_game/registry.h>

#include <voxy/core/log.h>

#include <assert.h>
#include <stdlib.h>

static struct block_info block_infos[BLOCK_MAX];
static struct item_info  item_infos[ITEM_MAX];

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
      LOG_INFO("  ether          = %u", block_info.ether);
      LOG_INFO("  light level    = %u", block_info.light_level);
      LOG_INFO("  texture left   = %s", block_info.textures[BLOCK_FACE_LEFT]);
      LOG_INFO("  texture right  = %s", block_info.textures[BLOCK_FACE_RIGHT]);
      LOG_INFO("  texture back   = %s", block_info.textures[BLOCK_FACE_BACK]);
      LOG_INFO("  texture front  = %s", block_info.textures[BLOCK_FACE_FRONT]);
      LOG_INFO("  texture bottom = %s", block_info.textures[BLOCK_FACE_BOTTOM]);
      LOG_INFO("  texture top    = %s", block_info.textures[BLOCK_FACE_TOP]);

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
