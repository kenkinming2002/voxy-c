#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include "info.h"

typedef uint8_t block_id_t;

struct block_registry
{
  struct block_infos infos;
};

void block_registry_init(struct block_registry *registry);
void block_registry_fini(struct block_registry *registry);

block_id_t block_registry_register_block(struct block_registry *registry, struct block_info block_info);
struct block_info block_registry_query_block(struct block_registry *registry, block_id_t id);

#endif // BLOCK_REGISTRY_H
