#ifndef VOXY_PLAYER_H
#define VOXY_PLAYER_H

#include <types/entity.h>
#include <types/inventory.h>
#include <types/hotbar.h>

#include <voxy/math/vector.h>

#include <stdbool.h>

struct player_entity
{
  struct entity base;

  bool spawned;
  bool third_person;

  struct inventory inventory;
  struct hotbar    hotbar;
  struct item     *item_hovered;
  struct item      item_held;
  fvec2_t          item_held_position;

  float cooldown;

  bool has_target_destroy;
  bool has_target_place;

  ivec3_t target_destroy;
  ivec3_t target_place;
};


#endif // VOXY_PLAYER_H
