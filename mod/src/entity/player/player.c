#include "mod.h"
#include "player.h"

#include "actions.h"
#include "camera_follow.h"
#include "chunk_loader.h"
#include "controls.h"
#include "inventory.h"

#include <voxy/scene/main_game/render/debug.h>
#include <voxy/core/window.h>
#include <voxy/core/log.h>

#include <stdlib.h>

static entity_id_t player_entity_id = ENTITY_NONE;

void player_entity_register(void)
{
  struct entity_info entity_info;

  entity_info.mod = MOD;
  entity_info.name = "player";

  entity_info.mesh = NULL;
  entity_info.texture = NULL;

  entity_info.hitbox_dimension = fvec3(0.8f, 0.8f, 2.0f);
  entity_info.hitbox_offset = fvec3(0.0f, 0.0f, -0.5f);

  entity_info.on_dispose = player_entity_fini;

  entity_info.on_save = player_entity_save;
  entity_info.on_load = player_entity_load;

  entity_info.on_update = player_entity_update;

  player_entity_id = register_entity_info(entity_info);
}

entity_id_t player_entity_id_get(void)
{
  return player_entity_id;
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

  entity->id = player_entity_id;
  entity->opaque = opaque;
}

void player_entity_fini(struct entity *entity) { free(entity->opaque); }

bool player_entity_save(const struct entity *entity, FILE *file)
{
  const struct player_opaque *opaque = entity->opaque;

#define SAVE(x) if(fwrite(&x, sizeof x, 1, file) != 1) return false;
  SAVE(opaque->inventory);
  SAVE(opaque->hotbar);
  SAVE(opaque->hand);
  SAVE(opaque->hotbar_selection);
  SAVE(opaque->cooldown);
  SAVE(opaque->cooldown_weird);
#undef SAVE

  return true;
}

bool player_entity_load(struct entity *entity, FILE *file)
{
  struct player_opaque *opaque = malloc(sizeof *opaque);
  entity->opaque = opaque;

  opaque->inventory_opened = false;
  opaque->third_person = false;

#define LOAD(x) if(fread(&x, sizeof x, 1, file) != 1) return false;
  LOAD(opaque->inventory);
  LOAD(opaque->hotbar);
  LOAD(opaque->hand);
  LOAD(opaque->hotbar_selection);
  LOAD(opaque->cooldown);
  LOAD(opaque->cooldown_weird);
#undef LOAD

  return true;
}

void player_entity_update(struct entity *entity, float dt)
{
  player_entity_update_actions(entity, dt);
  player_entity_update_camera_follow(entity, dt);
  player_entity_update_chunk_loader(entity);
  player_entity_update_controls(entity, dt);
  player_entity_update_inventory(entity);

  // Miscellenous controls
  // FIXME: That should not probably not be here
  if(input_press(KEY_P))
    main_game_render_set_debug(!main_game_render_get_debug());
}
