#include <voxy/scene/main_game/states/digger.h>
#include <voxy/scene/main_game/states/chunks.h>

struct digger g_digger = {
  .position = {0},
  .damage = 0.0f,
};

void digger_reset(struct digger *digger)
{
  if(digger->damage != 0.0f)
  {
    world_invalidate_mesh_at(digger->position);
    digger->position = ivec3_zero();
    digger->damage = 0.0f;
  }
}

void digger_set_position(struct digger *digger, ivec3_t position)
{
  if(!ivec3_eql(digger->position, position))
  {
    world_invalidate_mesh_at(digger->position);
    digger->position = position;
    digger->damage = 0.0f;
  }
}

bool digger_dig(struct digger *digger, float damage)
{
  world_invalidate_mesh_at(digger->position);
  digger->damage += damage;
  return digger->damage >= 1.0f;
}
