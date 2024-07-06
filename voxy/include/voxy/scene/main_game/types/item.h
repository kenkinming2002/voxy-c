#ifndef VOXY_MAIN_GAME_TYPES_ITEM_H
#define VOXY_MAIN_GAME_TYPES_ITEM_H

#include <voxy/scene/main_game/types/registry.h>
#include <stdint.h>

/// Maximum stack size of item.
///
/// FIXME: Consider adding a per item type stack sise limit in struct item_info.
#define ITEM_MAX_STACK 64

/// This representation of "a stack of items" rather than "item", and maybe I
/// should rename it. While manipulating this structure, you should make sure
/// that if count is 0, then id should ITEM_NONE, because have 0 of any
/// particular item does not make sense.
///
/// TODO: Consider adding a implied offset of 1 to item count.
struct item
{
  item_id_t id;
  uint8_t count;
};

/// Try to merge item from source to target. Only work if target and source have
/// same id or target is empty (i.e. id is ITEM_NONE). Otherwise, this is a
/// no-op.
void item_merge(struct item* target, struct item *source);

#endif // VOXY_MAIN_GAME_TYPES_ITEM_H
