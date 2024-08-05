#include "ether.h"
#include "mod.h"

static block_id_t ether_block_id;

void ether_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.type = BLOCK_TYPE_INVISIBLE;
  block_info.light_level = 15;

  block_info.on_create = ether_block_on_create;
  block_info.on_destroy = ether_block_on_destroy;

  ether_block_id = register_block_info(block_info);
}

block_id_t ether_block_id_get(void)
{
  return ether_block_id;
}

void ether_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void ether_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

