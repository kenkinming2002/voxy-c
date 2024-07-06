#include "block.h"
#include "ids.h"

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/entity/item.h>

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

#include <voxy/core/log.h>

static item_id_t block_id_to_item_id(block_id_t block_id)
{
  if(block_id == BLOCK_ID_STONE) return ITEM_ID_STONE;
  if(block_id == BLOCK_ID_GRASS) return ITEM_ID_GRASS;
  if(block_id == BLOCK_ID_LOG)   return ITEM_ID_LOG;
  if(block_id == BLOCK_ID_LEAVE) return ITEM_ID_LEAVE;
  if(block_id == BLOCK_ID_LAMP)  return ITEM_ID_LAMP;
  assert(0 && "Unreachable");
}

static ivec3_t block_local_position(struct chunk *chunk, struct block *block)
{
  // Black magic
  const int index = block - &chunk->data->blocks[0][0][0];
  const int x = index % CHUNK_WIDTH;
  const int y = (index / CHUNK_WIDTH) % CHUNK_WIDTH;
  const int z = (index / CHUNK_WIDTH / CHUNK_WIDTH) % CHUNK_WIDTH;
  return ivec3(x, y, z);
}

static ivec3_t block_global_position(struct chunk *chunk, struct block *block)
{
  return ivec3_add(ivec3_mul_scalar(chunk->position, CHUNK_WIDTH), block_local_position(chunk, block));
}

void block_on_create(struct entity *entity, struct chunk *chunk, struct block *block)
{
  (void)entity;
  (void)chunk;
  (void)block;
}

void block_on_destroy(struct entity *entity, struct chunk *chunk, struct block *block)
{
  (void)entity;

  struct entity item_entity;
  item_entity.position = ivec3_as_fvec3(block_global_position(chunk, block));
  item_entity.velocity = fvec3_zero();
  item_entity.rotation = fvec3_zero();
  item_entity.grounded = false;
  item_entity_init(&item_entity, (struct item) { .id = block_id_to_item_id(block->id), .count = 1 });
  world_add_entity(item_entity);
}
