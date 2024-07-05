#include "block.h"
#include "ids.h"

#include <voxy/core/log.h>
#include <voxy/scene/main_game/types/block.h>

static item_id_t block_id_to_item_id(block_id_t block_id)
{
  if(block_id == BLOCK_ID_STONE) return BLOCK_ID_STONE;
  if(block_id == BLOCK_ID_GRASS) return BLOCK_ID_GRASS;
  if(block_id == BLOCK_ID_LOG)   return BLOCK_ID_LOG;
  if(block_id == BLOCK_ID_LEAVE) return BLOCK_ID_LEAVE;
  if(block_id == BLOCK_ID_LAMP)  return BLOCK_ID_LAMP;
  assert(0 && "Unreachable");
}

void block_on_create(struct entity *entity, struct chunk *chunk, struct block *block)
{
  LOG_INFO("Block with id %d created\n", block->id);
}

void block_on_destroy(struct entity *entity, struct chunk *chunk, struct block *block)
{
  LOG_INFO("Block with id %d destroyed\n", block->id);
}
