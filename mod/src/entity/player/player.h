#ifndef ENTITY_PLAYER_PLAYER_H
#define ENTITY_PLAYER_PLAYER_H

#include "config.h"

#include <voxy/scene/main_game/types/item.h>
#include <voxy/scene/main_game/types/entity.h>

#include <stdbool.h>

struct player_opaque
{
  struct item inventory[PLAYER_INVENTORY_SIZE_VERTICAL][PLAYER_INVENTORY_SIZE_HORIZONTAL];
  struct item hotbar[PLAYER_HOTBAR_SIZE];
  struct item hand;

  bool inventory_opened;
  uint8_t hotbar_selection;

  bool third_person;

  float cooldown;
  float cooldown_weird;
};

entity_id_t player_entity_id(void);

void player_entity_init(struct entity *entity);
void player_entity_fini(struct entity *entity);

void player_entity_update(struct entity *entity, float dt);

#endif // ENTITY_PLAYER_PLAYER_H
