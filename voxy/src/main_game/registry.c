#include <voxy/main_game/registry.h>

#include <assert.h>
#include <stdio.h>
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
      fprintf(stderr, "INFO: Registered block: id = %d:\n", i);
      fprintf(stderr, "INFO:   mod            = %s\n", block_info.mod);
      fprintf(stderr, "INFO:   name           = %s\n", block_info.name);
      fprintf(stderr, "INFO:   type           = %s\n", block_type_as_str(block_info.type));
      fprintf(stderr, "INFO:   ether          = %u\n", block_info.ether);
      fprintf(stderr, "INFO:   light level    = %u\n", block_info.light_level);
      fprintf(stderr, "INFO:   texture left   = %s\n", block_info.textures[BLOCK_FACE_LEFT]);
      fprintf(stderr, "INFO:   texture right  = %s\n", block_info.textures[BLOCK_FACE_RIGHT]);
      fprintf(stderr, "INFO:   texture back   = %s\n", block_info.textures[BLOCK_FACE_BACK]);
      fprintf(stderr, "INFO:   texture front  = %s\n", block_info.textures[BLOCK_FACE_FRONT]);
      fprintf(stderr, "INFO:   texture bottom = %s\n", block_info.textures[BLOCK_FACE_BOTTOM]);
      fprintf(stderr, "INFO:   texture top    = %s\n", block_info.textures[BLOCK_FACE_TOP]);

      block_infos[i] = block_info;
      return i;
    }

  fprintf(stderr, "ERROR: Failed to allocate block id\n");
  exit(EXIT_FAILURE);
}

item_id_t register_item_info(struct item_info item_info)
{
  for(int i=0; i<BLOCK_MAX; ++i)
    if(!item_infos[i].mod  && !item_infos[i].name)
    {
      fprintf(stderr, "INFO: Registered item: id = %d:\n", i);
      fprintf(stderr, "INFO:   mod     = %s\n", item_info.mod);
      fprintf(stderr, "INFO:   name    = %s\n", item_info.name);
      fprintf(stderr, "INFO:   texture = %s\n", item_info.texture);
      fprintf(stderr, "INFO:   on use  = %p\n", item_info.on_use);

      item_infos[i] = item_info;
      return i;
    }

  fprintf(stderr, "ERROR: Failed to allocate item id\n");
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
