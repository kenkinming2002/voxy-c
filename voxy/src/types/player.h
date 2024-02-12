#ifndef VOXY_PLAYER_H
#define VOXY_PLAYER_H

#include <types/entity.h>
#include <types/inventory.h>
#include <types/hotbar.h>

#include <voxy/math/vector.h>

#include <stdbool.h>

struct player_entity
{
  bool spawned;
  bool third_person;

  struct entity base;

  struct inventory inventory;
  struct hotbar    hotbar;
  struct item      item_held;
  fvec2_t          item_held_position;
  struct item     *item_hovered;

  float cooldown;
};


#endif // VOXY_PLAYER_H
