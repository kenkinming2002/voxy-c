#ifndef ENTITY_PLAYER_PLAYER_H
#define ENTITY_PLAYER_PLAYER_H

#include "config.h"

#include <voxy/scene/main_game/types/item.h>
#include <voxy/scene/main_game/types/entity.h>

#include <stdbool.h>

enum player_ui_state
{
  PLAYER_UI_STATE_DEFAULT,
  PLAYER_UI_STATE_INVENTORY_OPENED,
  PLAYER_UI_STATE_CONTAINER_OPENED,
};

struct player_opaque
{
  fvec3_t spawn_position;

  enum player_ui_state ui_state;

  struct item hotbar[PLAYER_HOTBAR_SIZE];
  uint8_t     hotbar_selection;

  struct item inventory[PLAYER_INVENTORY_SIZE_VERTICAL][PLAYER_INVENTORY_SIZE_HORIZONTAL];
  struct item crafting_inputs[3][3];

  struct container *container;

  struct item hand;

  bool third_person;

  float cooldown;
  float cooldown_weird;
};

void player_entity_register(void);
entity_id_t player_entity_id_get(void);

void player_entity_init(struct entity *entity);
void player_entity_fini(struct entity *entity);

int player_entity_serialize(const struct entity *entity, struct serializer *serializer);
int player_entity_deserialize(struct entity *entity, struct deserializer *deserializer);

void player_entity_update(struct entity *entity, float dt);
void player_entity_render(const struct entity *entity, const struct camera *camera);

#endif // ENTITY_PLAYER_PLAYER_H
