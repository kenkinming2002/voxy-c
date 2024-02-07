#include "world_update_physics.h"

#include "world.h"
#include "entity.h"

static void entity_physics_integrate(struct entity *entity, float dt)
{
  entity->position = fvec3_add(entity->position, fvec3_mul_scalar(entity->velocity, dt));
}

void world_update_physics(struct world *world, float dt)
{
  entity_physics_integrate(&world->player.base, dt);
}
