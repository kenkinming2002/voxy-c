#include "spawn_weird.h"
#include "../weird/weird.h"

#include <voxy/scene/main_game/states/chunks.h>

void player_entity_update_spawn_weird(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;
  for(opaque->cooldown_weird += dt; opaque->cooldown_weird >= 2.0f; opaque->cooldown_weird -= 2.0f)
  {
    struct entity new_entity;
    new_entity.position = entity->position;
    new_entity.velocity = entity->velocity;
    new_entity.rotation = entity->rotation;
    new_entity.grounded = entity->grounded;
    weird_entity_init(&new_entity);
    if(!world_add_entity(new_entity))
      weird_entity_fini(&new_entity);
  }
}
