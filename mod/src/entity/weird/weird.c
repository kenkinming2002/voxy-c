#include "weird.h"

#include <stdlib.h>

static float randf(float low, float high)
{
  return low + (float)rand() / (float)RAND_MAX * (high - low);
}

entity_id_t weird_entity_id(void)
{
  static entity_id_t id = ENTITY_NONE;
  if(id == ENTITY_NONE)
  {
    struct entity_info entity_info;
    entity_info.mod = "main";
    entity_info.name = "weird";
    entity_info.hitbox_dimension = fvec3(1.0f, 1.0f, 2.0f);
    entity_info.hitbox_offset = fvec3(0.0f, 0.0f, -0.5f);
    entity_info.on_update = weird_entity_update;
    id = register_entity_info(entity_info);
  }
  return id;
}

void weird_entity_init(struct entity *entity)
{
  entity->id = weird_entity_id();
  entity->opaque = NULL;
}

void weird_entity_fini(struct entity *entity)
{
  (void)entity;
}

void weird_entity_update(struct entity *entity, float dt)
{
  entity_jump(entity, 20.0f);
  entity_move(entity, fvec2(randf(-1.0f, 1.0f), randf(-1.0f, 1.0f)), 30.0f, dt);
}



