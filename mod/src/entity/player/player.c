#include "player.h"

#include "actions.h"
#include "camera_follow.h"
#include "chunk_loader.h"
#include "controls.h"
#include "inventory.h"
#include "spawn_weird.h"

#include <voxy/scene/main_game/render/debug.h>
#include <voxy/core/window.h>

#include <stdlib.h>

entity_id_t player_entity_id(void)
{
  static entity_id_t id = ENTITY_NONE;
  if(id == ENTITY_NONE)
  {
    struct entity_info entity_info;
    entity_info.mod = "main";
    entity_info.name = "player";
    entity_info.hitbox_dimension = fvec3(1.0f, 1.0f, 2.0f);
    entity_info.hitbox_offset = fvec3(0.0f, 0.0f, -0.5f);
    entity_info.on_update = player_entity_update;
    id = register_entity_info(entity_info);
  }
  return id;
}

void player_entity_init(struct entity *entity)
{
  struct player_opaque *opaque = malloc(sizeof *opaque);

  for(int j=0; j<PLAYER_INVENTORY_SIZE_VERTICAL; ++j)
    for(int i=0; i<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++i)
    {
      opaque->inventory[j][i].id = ITEM_NONE;
      opaque->inventory[j][i].count = 0;
    }

  for(int i=0; i<PLAYER_HOTBAR_SIZE; ++i)
    if(i < 5)
    {
      opaque->hotbar[i].id = i;
      opaque->hotbar[i].count = 8 * (i + 1);
    }
    else
    {
      opaque->hotbar[i].id = ITEM_NONE;
      opaque->hotbar[i].count = 0;
    }

  opaque->hand.id = ITEM_NONE;
  opaque->hand.count = 0;

  opaque->inventory_opened = false;
  opaque->hotbar_selection = 0;

  opaque->third_person = false;

  opaque->cooldown = 0.0f;
  opaque->cooldown_weird = 0.0f;

  entity->id = player_entity_id();
  entity->opaque = opaque;
}

void player_entity_fini(struct entity *entity)
{
  free(entity->opaque);
}

void player_entity_update(struct entity *entity, float dt)
{
  player_entity_update_actions(entity, dt);
  player_entity_update_camera_follow(entity, dt);
  player_entity_update_chunk_loader(entity);
  player_entity_update_controls(entity, dt);
  player_entity_update_inventory(entity);
  player_entity_update_spawn_weird(entity, dt);

  // Miscellenous controls
  // FIXME: That should not probably not be here
  if(input_press(KEY_P))
    main_game_render_set_debug(!main_game_render_get_debug());
}
