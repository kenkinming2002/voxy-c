#ifndef ITEM_ITEM_H
#define ITEM_ITEM_H

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

bool item_on_use_place_block(struct entity *entity, struct item *item, block_id_t block_id);

#endif // ITEM_ITEM_H
