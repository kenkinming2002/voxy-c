#include "mod.h"
#include "player.h"

#include "ui/layout.h"

#include "actions.h"
#include "camera_follow.h"
#include "chunk_loader.h"
#include "controls.h"
#include "inventory.h"
#include "health_ui.h"

#include <voxy/scene/main_game/render/debug.h>
#include <voxy/scene/main_game/render/debug_overlay.h>

#include <libcommon/core/window.h>
#include <libcommon/core/log.h>

#include <stdlib.h>

static entity_id_t player_entity_id = ENTITY_NONE;

void player_entity_register(void)
{
  struct entity_info entity_info;

  entity_info.mod = MOD;
  entity_info.name = "player";

  entity_info.hitbox_dimension = fvec3(0.8f, 0.8f, 1.9f);
  entity_info.hitbox_offset = fvec3(0.0f, 0.0f, -0.5f);

  entity_info.on_dispose = player_entity_fini;

  entity_info.serialize = player_entity_serialize;
  entity_info.deserialize = player_entity_deserialize;

  entity_info.on_update = player_entity_update;
  entity_info.on_render = player_entity_render;

  player_entity_id = register_entity_info(entity_info);
}

entity_id_t player_entity_id_get(void)
{
  return player_entity_id;
}

void player_entity_init(struct entity *entity)
{
  struct player_opaque *opaque = malloc(sizeof *opaque);

  opaque->spawn_position = entity->position;

  for(int i=0; i<PLAYER_HOTBAR_SIZE; ++i)
    if(i < 9)
    {
      opaque->hotbar[i].id = i;
      opaque->hotbar[i].count = 4 * (i + 1);
    }
    else
    {
      opaque->hotbar[i].id = ITEM_NONE;
      opaque->hotbar[i].count = 0;
    }

  for(int j=0; j<PLAYER_INVENTORY_SIZE_VERTICAL; ++j)
    for(int i=0; i<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++i)
    {
      opaque->inventory[j][i].id = ITEM_NONE;
      opaque->inventory[j][i].count = 0;
    }

  for(int j=0; j<3; ++j)
    for(int i=0; i<3; ++i)
    {
      opaque->crafting_inputs[j][i].id = ITEM_NONE;
      opaque->crafting_inputs[j][i].count = 0;
    }

  opaque->hand.id = ITEM_NONE;
  opaque->hand.count = 0;

  opaque->inventory_opened = false;
  opaque->hotbar_selection = 0;

  opaque->third_person = false;

  opaque->cooldown = 0.0f;
  opaque->cooldown_weird = 0.0f;

  entity->id = player_entity_id;
  entity->health = 10.0f;
  entity->max_health = 10.0f;
  entity->opaque = opaque;
}

void player_entity_fini(struct entity *entity) { free(entity->opaque); }

int player_entity_serialize(const struct entity *entity, struct serializer *serializer)
{
  const struct player_opaque *opaque = entity->opaque;

  SERIALIZE(serializer, opaque->spawn_position);
  SERIALIZE(serializer, opaque->hotbar);
  SERIALIZE(serializer, opaque->inventory);
  SERIALIZE(serializer, opaque->crafting_inputs);
  SERIALIZE(serializer, opaque->hand);
  SERIALIZE(serializer, opaque->hotbar_selection);
  SERIALIZE(serializer, opaque->cooldown);
  SERIALIZE(serializer, opaque->cooldown_weird);

  return 0;
}

int player_entity_deserialize(struct entity *entity, struct deserializer *deserializer)
{
  struct player_opaque *opaque = malloc(sizeof *opaque);
  entity->opaque = opaque;

  DESERIALIZE(deserializer, opaque->spawn_position);
  DESERIALIZE(deserializer, opaque->hotbar);
  DESERIALIZE(deserializer, opaque->inventory);
  DESERIALIZE(deserializer, opaque->crafting_inputs);
  DESERIALIZE(deserializer, opaque->hand);
  DESERIALIZE(deserializer, opaque->hotbar_selection);
  DESERIALIZE(deserializer, opaque->cooldown);
  DESERIALIZE(deserializer, opaque->cooldown_weird);

  opaque->inventory_opened = false;
  opaque->third_person = false;

  return 0;
}

void player_entity_update(struct entity *entity, float dt)
{
  const struct player_ui_layout ui_layout = compute_player_ui_layout();

  player_entity_update_actions(entity, dt);
  player_entity_update_camera_follow(entity);
  player_entity_update_chunk_loader(entity);
  player_entity_update_controls(entity, dt);
  player_entity_update_inventory(entity, dt, ui_layout);
  player_entity_update_health_ui(entity, ui_layout);

  // Miscellenous controls
  // FIXME: That should not probably not be here
  if(input_press(KEY_P))
    main_game_render_set_debug(!main_game_render_get_debug());

  main_game_debug_overlay_printf("Position: %f %f %f", entity->position.x, entity->position.y, entity->position.z);
  main_game_debug_overlay_printf("Velocity: %f %f %f", entity->velocity.x, entity->velocity.y, entity->velocity.z);
  main_game_debug_overlay_printf("Rotation: %f %f (Yaw Pitch)", entity->rotation.yaw, entity->rotation.pitch);
}

void player_entity_render(const struct entity *entity, const struct camera *camera)
{
  (void)entity;
  (void)camera;
}

